using System.ComponentModel;
using System.Text;

namespace Kiwi.Native;

/// <summary>
/// Entry points for the code the source generator writes. Not meant to be called directly.
/// </summary>
[EditorBrowsable(EditorBrowsableState.Never)]
public static unsafe class KiwiRuntime
{
    /// <summary>The <c>KIWI_ABI_VERSION</c> this SDK targets.</summary>
    public const uint AbiVersion = 1;

    internal static KiwiApi* Api;

    /// <summary>Keeps the host's function table, or returns false if the host is too old.</summary>
    public static bool Initialize(void* api)
    {
        var table = (KiwiApi*)api;
        if (table == null || table->AbiVersion < AbiVersion || table->Size < (uint)sizeof(KiwiApi))
            return false;
        Api = table;
        return true;
    }

    public static bool Define(nint ctx, string name, delegate* unmanaged[Cdecl]<nint, int, ulong*, ulong> fn, int arity)
    {
        var utf8 = NulTerminated(name);
        fixed (byte* p = utf8)
            return Api->Define(ctx, p, (nint)fn, arity) == 0;
    }

    public static KiwiContext Enter(nint ctx) => new(ctx);

    /// <summary>Throws unless <paramref name="argc"/> is within the bounds; a negative max means no limit.</summary>
    public static void CheckArgumentCount(string function, int argc, int min, int max)
    {
        if (argc >= min && (max < 0 || argc <= max))
            return;

        var (expected, count) = max < 0 ? ($"at least {min}", min)
                              : min == max ? ($"{min}", min)
                              : ($"{min} to {max}", max);
        throw new KiwiException($"{function} expects {expected} argument{(count == 1 ? "" : "s")}, got {argc}.");
    }

    /// <summary>A non-null value for an <c>object</c> parameter.</summary>
    public static object NotNull(object? value) =>
        value ?? throw new KiwiException("expected a value, got null.");

    /// <summary>
    /// Reports an exception to Kiwi as a <c>NativeError</c> and returns the handle to return,
    /// 0. A conversion error names the argument it was converting.
    /// </summary>
    public static ulong Fail(nint ctx, string function, int argument, string? parameter, Exception error)
    {
        var message = argument >= 0 && error is KiwiException
            ? $"{function}: argument {argument + 1} ({parameter}): {error.Message}"
            : error.Message;
        Raise(ctx, message);
        return 0;
    }

    /// <summary>Reports an initialization failure.</summary>
    public static int FailInit(nint ctx, Exception error)
    {
        if (Api != null)
            Raise(ctx, error.Message);
        return 1;
    }

    private static void Raise(nint ctx, string message)
    {
        var utf8 = NulTerminated(message);
        fixed (byte* p = utf8)
            Api->Raise(ctx, p);
    }

    private static byte[] NulTerminated(string s)
    {
        var bytes = new byte[Encoding.UTF8.GetByteCount(s) + 1];
        Encoding.UTF8.GetBytes(s, 0, s.Length, bytes, 0);
        return bytes;
    }
}
