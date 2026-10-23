using System.Runtime.InteropServices;

namespace kiwi.Runtime.Ffi;

/// <summary>
/// Calls a C function with any mix of integer, pointer and floating-point arguments without
/// generating code, so it works under native AOT.
/// <para>
/// Each argument arrives as a 64-bit slot: an integer or pointer extended to 64 bits, or the
/// bits of a double (a float in the low 32 bits). The C calling conventions Kiwi runs on pass
/// these in a few fixed ways, so a handful of function pointer signatures cover them all:
/// </para>
/// <list type="bullet">
/// <item>System V x64 (Linux, macOS on Intel): integer arguments fill six registers and
/// floating-point ones eight, independently; the rest go on the stack in order, 8 bytes each.</item>
/// <item>arm64 (Linux, macOS): the same with eight integer registers. macOS packs stack
/// arguments by size, so there only 8-byte arguments may overflow to the stack.</item>
/// <item>Windows x64: the first four arguments go in a register chosen by position, integer or
/// floating-point by type; the rest go on the stack, 8 bytes each.</item>
/// </list>
/// <para>
/// A callee ignores registers and stack slots it doesn't use, and the caller removes the stack
/// arguments, so passing the unused ones as zero is safe. Structs passed by value and variadic
/// functions are not supported.
/// </para>
/// </summary>
public static unsafe partial class FfiInvoker
{
    private const int StackSlots = 8;

    private enum Convention { SysV, Arm64, AppleArm64, Windows, Unsupported }

    private static readonly Convention Abi = RuntimeInformation.ProcessArchitecture switch
    {
        Architecture.X64 when OperatingSystem.IsWindows() => Convention.Windows,
        Architecture.X64                                  => Convention.SysV,
        Architecture.Arm64 when OperatingSystem.IsMacOS() => Convention.AppleArm64,
        Architecture.Arm64 when !OperatingSystem.IsWindows() => Convention.Arm64,
        _                                                 => Convention.Unsupported,
    };

    /// <summary>
    /// Calls <paramref name="fn"/>. Returns the integer result, or the bits of a floating-point
    /// one, as 64 bits.
    /// </summary>
    /// <exception cref="InvalidOperationException">The platform or the arguments aren't supported.</exception>
    public static ulong Invoke(nint fn, ReadOnlySpan<ulong> args, ReadOnlySpan<FfiType> types, FfiType returns)
    {
        var floatReturn = returns.IsFloatingPoint();
        return Abi switch
        {
            Convention.SysV       => CallSplit(fn, args, types, floatReturn, intRegisters: 6, apple: false),
            Convention.Arm64      => CallSplit(fn, args, types, floatReturn, intRegisters: 8, apple: false),
            Convention.AppleArm64 => CallSplit(fn, args, types, floatReturn, intRegisters: 8, apple: true),
            Convention.Windows    => CallWindows(fn, args, types, floatReturn),
            _ => throw new InvalidOperationException(
                $"ffi doesn't support {RuntimeInformation.OSDescription} on {RuntimeInformation.ProcessArchitecture}."),
        };
    }

    // System V x64 and arm64: integer and floating-point registers fill independently.
    private static ulong CallSplit(nint fn, ReadOnlySpan<ulong> args, ReadOnlySpan<FfiType> types, bool floatReturn, int intRegisters, bool apple)
    {
        var r = stackalloc ulong[8];
        var f = stackalloc ulong[8];
        var s = stackalloc ulong[StackSlots];
        int ni = 0, nf = 0, ns = 0;

        for (var k = 0; k < args.Length; k++)
        {
            if (types[k].IsFloatingPoint() ? nf < 8 : ni < intRegisters)
            {
                if (types[k].IsFloatingPoint()) f[nf++] = args[k];
                else r[ni++] = args[k];
                continue;
            }

            if (apple && types[k].Size() != 8)
                throw new InvalidOperationException(
                    $"On macOS arm64, only 8-byte arguments can follow the first 8 integer and 8 floating-point arguments; argument {k + 1} is {types[k].Name()}.");
            if (ns == StackSlots)
                throw TooMany(args.Length);
            s[ns++] = args[k];
        }

        if (intRegisters == 6)
        {
            if (floatReturn)
                return Bits(((delegate* unmanaged<nint, nint, nint, nint, nint, nint, double, double, double, double, double, double, double, double, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)(
                    (nint)r[0], (nint)r[1], (nint)r[2], (nint)r[3], (nint)r[4], (nint)r[5],
                    D(f[0]), D(f[1]), D(f[2]), D(f[3]), D(f[4]), D(f[5]), D(f[6]), D(f[7]),
                    (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]));
            return (ulong)((delegate* unmanaged<nint, nint, nint, nint, nint, nint, double, double, double, double, double, double, double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)(
                (nint)r[0], (nint)r[1], (nint)r[2], (nint)r[3], (nint)r[4], (nint)r[5],
                D(f[0]), D(f[1]), D(f[2]), D(f[3]), D(f[4]), D(f[5]), D(f[6]), D(f[7]),
                (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]);
        }

        if (floatReturn)
            return Bits(((delegate* unmanaged<nint, nint, nint, nint, nint, nint, nint, nint, double, double, double, double, double, double, double, double, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)(
                (nint)r[0], (nint)r[1], (nint)r[2], (nint)r[3], (nint)r[4], (nint)r[5], (nint)r[6], (nint)r[7],
                D(f[0]), D(f[1]), D(f[2]), D(f[3]), D(f[4]), D(f[5]), D(f[6]), D(f[7]),
                (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]));
        return (ulong)((delegate* unmanaged<nint, nint, nint, nint, nint, nint, nint, nint, double, double, double, double, double, double, double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)(
            (nint)r[0], (nint)r[1], (nint)r[2], (nint)r[3], (nint)r[4], (nint)r[5], (nint)r[6], (nint)r[7],
            D(f[0]), D(f[1]), D(f[2]), D(f[3]), D(f[4]), D(f[5]), D(f[6]), D(f[7]),
            (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]);
    }

    // Windows x64: the first four arguments take the register for their position.
    private static ulong CallWindows(nint fn, ReadOnlySpan<ulong> args, ReadOnlySpan<FfiType> types, bool floatReturn)
    {
        if (args.Length > 4 + StackSlots)
            throw TooMany(args.Length);

        var r = stackalloc ulong[4];
        var s = stackalloc ulong[StackSlots];
        var mask = 0;
        for (var k = 0; k < args.Length; k++)
        {
            if (k < 4)
            {
                r[k] = args[k];
                if (types[k].IsFloatingPoint()) mask |= 1 << k;
            }
            else
            {
                s[k - 4] = args[k];
            }
        }

        return floatReturn ? Bits(CallWindowsFloat(fn, mask, r, s)) : (ulong)CallWindowsInt(fn, mask, r, s);
    }

    private static InvalidOperationException TooMany(int count) =>
        new($"ffi can't pass {count} arguments on this platform.");

    private static double D(ulong bits) => BitConverter.Int64BitsToDouble((long)bits);

    private static ulong Bits(double d) => (ulong)BitConverter.DoubleToInt64Bits(d);
}
