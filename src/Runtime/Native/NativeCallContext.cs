using System.Runtime.InteropServices;
using kiwi.Parsing;
using kiwi.Typing;
using kiwi.VM;

namespace kiwi.Runtime.Native;

/// <summary>
/// The state behind a <c>KiwiCtx*</c>: the handle table for one native call, the
/// unmanaged buffers handed out during it, and any error it raised.
/// </summary>
public sealed class NativeCallContext
{
    [ThreadStatic] private static Stack<NativeCallContext>? _pool;

    private readonly List<Value> _values = [];
    private readonly List<nint> _buffers = [];
    private GCHandle _self;

    /// <summary>The value passed to native code as <c>KiwiCtx*</c>.</summary>
    public nint Handle { get; }

    public Token Token { get; private set; } = Token.Eof;
    public KiwiVM? VM { get; private set; }

    /// <summary>The extension being initialized, or null during an ordinary call.</summary>
    public NativeExtension? Initializing { get; private set; }

    /// <summary>The first error raised during the call, rethrown once it returns.</summary>
    public Exception? Error { get; private set; }

    private NativeCallContext()
    {
        // Weak so a context dropped with its thread's pool can still be collected.
        _self  = GCHandle.Alloc(this, GCHandleType.Weak);
        Handle = GCHandle.ToIntPtr(_self);
    }

    ~NativeCallContext()
    {
        if (_self.IsAllocated) _self.Free();
    }

    public static NativeCallContext Rent(Token token, KiwiVM? vm, NativeExtension? initializing = null)
    {
        _pool ??= new();
        var ctx = _pool.Count > 0 ? _pool.Pop() : new NativeCallContext();
        ctx.Token        = token;
        ctx.VM           = vm;
        ctx.Initializing = initializing;
        return ctx;
    }

    public void Return()
    {
        _values.Clear();
        foreach (var buffer in _buffers)
            Marshal.FreeHGlobal(buffer);
        _buffers.Clear();
        Error        = null;
        VM           = null;
        Initializing = null;
        _pool?.Push(this);
    }

    public static NativeCallContext FromHandle(nint handle)
    {
        return GCHandle.FromIntPtr(handle).Target as NativeCallContext
            ?? throw new InvalidOperationException("Invalid native call context.");
    }

    public ulong Add(Value value)
    {
        _values.Add(value);
        return (ulong)_values.Count;
    }

    public Value Get(ulong handle)
    {
        if (handle == 0 || handle > (ulong)_values.Count)
            throw new InvalidOperationException($"Invalid value handle {handle}.");
        return _values[(int)(handle - 1)];
    }

    /// <summary>Copies <paramref name="data"/> into a buffer that lives until the call returns.</summary>
    public unsafe byte* Pin(ReadOnlySpan<byte> data, bool nulTerminate)
    {
        var size   = data.Length + (nulTerminate ? 1 : 0);
        var buffer = Marshal.AllocHGlobal(Math.Max(size, 1));
        _buffers.Add(buffer);
        var dest = new Span<byte>((void*)buffer, size);
        data.CopyTo(dest);
        if (nulTerminate) dest[^1] = 0;
        return (byte*)buffer;
    }

    public void Fail(Exception error)
    {
        Error ??= error;
    }
}
