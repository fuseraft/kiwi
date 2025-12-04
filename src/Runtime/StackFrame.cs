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

public sealed class StackFrame
{
    public Scope Scope { get; }
    public Value? ReturnValue { get; set; }
    public InstanceRef? ObjectContext { get; private set; }
    public FrameFlags Flags { get; private set; } = FrameFlags.None;
    public string Name { get; }

    public StackFrame(string name, Scope scope)
    {
        Name   = name;
        Scope  = scope;
    }

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
}