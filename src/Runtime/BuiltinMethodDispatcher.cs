using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Handler;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime;

/// <summary>
/// Routes method-call AST nodes to the appropriate static builtin handler.
/// Extracted from Interpreter to reduce the god-class surface area.
/// </summary>
internal sealed class BuiltinMethodDispatcher
{
    private readonly Interpreter _interp;

    internal BuiltinMethodDispatcher(Interpreter interp)
    {
        _interp = interp;
    }

    internal Value HandleObjectBuiltin(MethodCallNode node, InstanceRef obj, string baseStruct, TokenName builtin)
    {
        var token = node.Token;
        List<Value> args = [];
        foreach (var arg in node.Arguments)
        {
            var value = _interp.Interpret(arg);
            args.Add(value);
        }

        return ObjectBuiltinHandler.Handle(_interp, token, builtin, obj, baseStruct, args);
    }

    internal Value HandleCallableBuiltin(MethodCallNode node, List<Value> args)
    {
        var obj = _interp.Interpret(node.Object);

        Callable? callable = null;
        string? callableName = null;
        if (obj.IsLambda())
        {
            callableName = obj.GetLambda().Identifier;

            if (_interp.Context.HasLambda(callableName))
            {
                callable = _interp.Context.Lambdas[callableName];
            }
            else if (_interp.Context.HasMappedLambda(callableName))
            {
                callable = _interp.Context.Lambdas[_interp.Context.LambdaTable[callableName]];
            }
        }
        else if (node.Object?.Type == ASTNodeType.Identifier)
        {
            callableName = ((IdentifierNode)node.Object).Name;

            if (_interp.Context.HasFunction(callableName))
            {
                callable = _interp.Context.Functions[callableName];
            }
        }

        if (callableName == null)
        {
            throw new InvalidOperationError(node.Token, $"Expected a callable for function `{CallableBuiltin.MapName(node.Op)}`.");
        }

        if (callable == null)
        {
            throw new InvalidOperationError(node.Token, $"Expected a callable for function `{CallableBuiltin.MapName(node.Op)}` on `{callableName}`.");
        }

        if (callable is not KFunction && callable is not KLambda)
        {
            throw new InvalidOperationError(node.Token, $"Expected a function or lambda for function `{CallableBuiltin.MapName(node.Op)}` on `{callableName}`.");
        }

        return CallableBuiltinHandler.Execute(_interp, node.Token, node.Op, callable, callableName, args);
    }

    internal Value HandleCallableBuiltinDirect(Token token, Value obj, string methodName, List<Value> args)
    {
        if (!CallableBuiltin.Map.TryGetValue(methodName, out var op))
            throw new FunctionUndefinedError(token, methodName);
        var argNodes = args.Select(a => (ASTNode?)new LiteralNode(a) { Token = token }).ToList();
        var node = new MethodCallNode(new LiteralNode(obj) { Token = token }, methodName, op, argNodes)
        { 
            Token = token 
        };
        return HandleCallableBuiltin(node, args);
    }
}
