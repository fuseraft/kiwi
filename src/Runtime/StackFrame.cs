using kiwi.Parsing;
using kiwi.Tracing;
using kiwi.Typing;

namespace kiwi.Runtime;

[Flags]
public enum FrameFlags : ushort
{
    None       = 0,
    Return     = 1 << 0,
    SubFrame   = 1 << 1,
    InLoop     = 1 << 2,
    Break      = 1 << 3,
    Next       = 1 << 4,
    InTry      = 1 << 5,
    InObject   = 1 << 6,
    InLambda   = 1 << 7,
}

public sealed class StackFrame(string name, Scope scope, Token? callSiteToken = null)
{
    public Scope Scope { get; set; } = scope;
    public Value? ReturnValue { get; set; }
    public InstanceRef? ObjectContext { get; private set; }
    public FrameFlags Flags { get; private set; } = FrameFlags.None;
    public string Name { get; } = name;
    public Token? CallSiteToken { get; set; } = callSiteToken;
    public bool IsFunction { get; set; } = false;
    public HashSet<string> LocalFunctions { get; } = [];
    public HashSet<string> LocalLambdas { get; } = [];

    public string StructName { get; set; } = string.Empty;

    public void SetObjectContext(InstanceRef? obj)
    {
        ObjectContext = obj;
        SetFlag(FrameFlags.InObject);
    }

    public bool InObjectContext() => IsFlagSet(FrameFlags.InObject);
    public InstanceRef? GetObjectContext() => ObjectContext;

    public void SetFlag(FrameFlags flag)   => Flags |= flag;
    public void ClearFlag(FrameFlags flag) => Flags &= ~flag;
    public bool IsFlagSet(FrameFlags flag) => (Flags & flag) == flag;

    public string FormatTraceLine()
    {
        var token = CallSiteToken ?? Token.Eof;
        var location = CallSiteToken != null 
            ? $"{FileRegistry.Instance.GetFilePath(token.Span.File)}:{token.Span.Line}:{token.Span.Pos}" 
            : "<unknown location>";
        
        if (Name == "<kiwi>")
        {
            return "Stack:";
        }

        return $"at {Name} in {location}";
    }
}