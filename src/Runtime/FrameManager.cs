using kiwi.Parsing;
using kiwi.Typing;

namespace kiwi.Runtime;

/// <summary>
/// Manages the call stack and scope lifetime for the interpreter.
/// Extracted from Interpreter to reduce the god-class surface area.
/// </summary>
internal sealed class FrameManager
{
    private readonly Stack<StackFrame> _callStack;
    private readonly Stack<string> _funcStack;
    private readonly Stack<Scope> _scopePool;
    private readonly Scope _globalScope;
    // Delegate instead of a direct reference so that SetContext() swaps are visible.
    private readonly Func<KContext> _getContext;

    internal FrameManager(
        Stack<StackFrame> callStack,
        Stack<string> funcStack,
        Stack<Scope> scopePool,
        Scope globalScope,
        Func<KContext> getContext)
    {
        _callStack = callStack;
        _funcStack = funcStack;
        _scopePool = scopePool;
        _globalScope = globalScope;
        _getContext = getContext;
    }

    internal List<string> CaptureStackTrace()
    {
        var trace = new List<string>();
        foreach (var frame in _callStack.Reverse())
        {
            trace.Add(frame.FormatTraceLine());
        }
        return trace;
    }

    internal StackFrame Push(StackFrame frame, bool inLambda = false)
    {
        if (inLambda)
        {
            frame.SetFlag(FrameFlags.InLambda);
        }

        _callStack.Push(frame);
        _funcStack.Push(frame.Name);
        return frame;
    }

    internal StackFrame Push(string name, Token token, Scope scope, bool inLambda = false)
    {
        var frame = new StackFrame(name, scope, token);

        if (inLambda)
        {
            frame.SetFlag(FrameFlags.InLambda);
        }

        _callStack.Push(frame);
        _funcStack.Push(name);
        return frame;
    }

    /// <summary>Raw push without inLambda support. Returns true for use in conditional patterns.</summary>
    internal bool PushRaw(StackFrame frame)
    {
        _callStack.Push(frame);
        _funcStack.Push(frame.Name);
        return true;
    }

    internal Value Pop()
    {
        if (_callStack.Count == 0)
        {
            return Value.Default;
        }

        var frame = _callStack.Pop();
        _funcStack.Pop();

        if (frame.IsFunction)
        {
            var ctx = _getContext();
            foreach (var fname in frame.LocalFunctions)
                ctx.Functions.Remove(fname);
            foreach (var lname in frame.LocalLambdas)
                ctx.Lambdas.Remove(lname);
        }

        var ret = frame.ReturnValue ?? Value.Default;
        if (_callStack.Count > 0)
        {
            _callStack.Peek().ReturnValue = ret;
        }

        return ret;
    }

    /// <summary>
    /// Enters a block scope by renting a Scope from the pool (or allocating a new one)
    /// and pointing the frame at it. Returns the previous (parent) scope so the caller
    /// can restore it in finally.
    /// </summary>
    internal Scope EnterBlock(StackFrame frame)
    {
        var parent = frame.Scope;
        Scope block = _scopePool.Count > 0 ? _scopePool.Pop() : new Scope();
        block.Reset(parent);
        frame.Scope = block;
        return parent;
    }

    /// <summary>
    /// Restores the frame to the scope saved by EnterBlock and returns the block scope
    /// to the pool if it was not captured by a lambda or nested function.
    /// </summary>
    internal void ExitBlock(StackFrame frame, Scope parent)
    {
        var block = frame.Scope;
        frame.Scope = parent;
        if (block.CanPool)
            _scopePool.Push(block);
    }

    /// <summary>
    /// Returns the current frame's scope and marks it (and its parent block scopes) as
    /// captured so they are never returned to the pool. Call whenever a lambda or function
    /// definition closes over the current scope.
    /// </summary>
    internal Scope CaptureCurrentScope()
    {
        var scope = _callStack.Peek().Scope;
        scope.MarkCaptured();
        return scope;
    }

    internal bool InTry()
    {
        if (_callStack.Count == 0)
        {
            return false;
        }

        return _callStack.Peek().IsFlagSet(FrameFlags.InTry);
    }

    internal StackFrame CreateFrame(string name, Token token, Scope? parentScope = null)
    {
        StackFrame frame = _callStack.Peek();
        Scope scope = new(parentScope ?? _globalScope);
        StackFrame subFrame = new(name, scope, token);

        if (frame.InObjectContext())
        {
            var objectContext = frame.GetObjectContext();
            subFrame.SetObjectContext(objectContext);
        }

        if (frame.IsFlagSet(FrameFlags.InTry))
        {
            subFrame.SetFlag(FrameFlags.InTry);
        }

        if (frame.IsFlagSet(FrameFlags.SubFrame))
        {
            subFrame.SetFlag(FrameFlags.SubFrame);
        }

        return subFrame;
    }
}
