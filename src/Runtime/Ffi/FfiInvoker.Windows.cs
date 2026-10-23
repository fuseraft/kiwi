// One case per mix of integer and float types in the four register positions; see CallWindows in FfiInvoker.cs.
namespace kiwi.Runtime.Ffi;

public static unsafe partial class FfiInvoker
{
    // Windows x64, returning an integer or pointer. Bit k of `mask` is set when register argument k is a float or double.
    private static nint CallWindowsInt(nint fn, int mask, ulong* r, ulong* s) => mask switch
    {
        0 => ((delegate* unmanaged<nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)((nint)r[0], (nint)r[1], (nint)r[2], (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        1 => ((delegate* unmanaged<double, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)(D(r[0]), (nint)r[1], (nint)r[2], (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        2 => ((delegate* unmanaged<nint, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)((nint)r[0], D(r[1]), (nint)r[2], (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        3 => ((delegate* unmanaged<double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)(D(r[0]), D(r[1]), (nint)r[2], (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        4 => ((delegate* unmanaged<nint, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)((nint)r[0], (nint)r[1], D(r[2]), (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        5 => ((delegate* unmanaged<double, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)(D(r[0]), (nint)r[1], D(r[2]), (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        6 => ((delegate* unmanaged<nint, double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)((nint)r[0], D(r[1]), D(r[2]), (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        7 => ((delegate* unmanaged<double, double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)(D(r[0]), D(r[1]), D(r[2]), (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        8 => ((delegate* unmanaged<nint, nint, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)((nint)r[0], (nint)r[1], (nint)r[2], D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        9 => ((delegate* unmanaged<double, nint, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)(D(r[0]), (nint)r[1], (nint)r[2], D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        10 => ((delegate* unmanaged<nint, double, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)((nint)r[0], D(r[1]), (nint)r[2], D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        11 => ((delegate* unmanaged<double, double, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)(D(r[0]), D(r[1]), (nint)r[2], D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        12 => ((delegate* unmanaged<nint, nint, double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)((nint)r[0], (nint)r[1], D(r[2]), D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        13 => ((delegate* unmanaged<double, nint, double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)(D(r[0]), (nint)r[1], D(r[2]), D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        14 => ((delegate* unmanaged<nint, double, double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)((nint)r[0], D(r[1]), D(r[2]), D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        15 => ((delegate* unmanaged<double, double, double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint>)fn)(D(r[0]), D(r[1]), D(r[2]), D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        _ => throw new ArgumentOutOfRangeException(nameof(mask)),
    };

    // Windows x64, returning a float or double. Bit k of `mask` is set when register argument k is a float or double.
    private static double CallWindowsFloat(nint fn, int mask, ulong* r, ulong* s) => mask switch
    {
        0 => ((delegate* unmanaged<nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)((nint)r[0], (nint)r[1], (nint)r[2], (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        1 => ((delegate* unmanaged<double, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)(D(r[0]), (nint)r[1], (nint)r[2], (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        2 => ((delegate* unmanaged<nint, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)((nint)r[0], D(r[1]), (nint)r[2], (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        3 => ((delegate* unmanaged<double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)(D(r[0]), D(r[1]), (nint)r[2], (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        4 => ((delegate* unmanaged<nint, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)((nint)r[0], (nint)r[1], D(r[2]), (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        5 => ((delegate* unmanaged<double, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)(D(r[0]), (nint)r[1], D(r[2]), (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        6 => ((delegate* unmanaged<nint, double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)((nint)r[0], D(r[1]), D(r[2]), (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        7 => ((delegate* unmanaged<double, double, double, nint, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)(D(r[0]), D(r[1]), D(r[2]), (nint)r[3], (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        8 => ((delegate* unmanaged<nint, nint, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)((nint)r[0], (nint)r[1], (nint)r[2], D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        9 => ((delegate* unmanaged<double, nint, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)(D(r[0]), (nint)r[1], (nint)r[2], D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        10 => ((delegate* unmanaged<nint, double, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)((nint)r[0], D(r[1]), (nint)r[2], D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        11 => ((delegate* unmanaged<double, double, nint, double, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)(D(r[0]), D(r[1]), (nint)r[2], D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        12 => ((delegate* unmanaged<nint, nint, double, double, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)((nint)r[0], (nint)r[1], D(r[2]), D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        13 => ((delegate* unmanaged<double, nint, double, double, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)(D(r[0]), (nint)r[1], D(r[2]), D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        14 => ((delegate* unmanaged<nint, double, double, double, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)((nint)r[0], D(r[1]), D(r[2]), D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        15 => ((delegate* unmanaged<double, double, double, double, nint, nint, nint, nint, nint, nint, nint, nint, double>)fn)(D(r[0]), D(r[1]), D(r[2]), D(r[3]), (nint)s[0], (nint)s[1], (nint)s[2], (nint)s[3], (nint)s[4], (nint)s[5], (nint)s[6], (nint)s[7]),
        _ => throw new ArgumentOutOfRangeException(nameof(mask)),
    };
}
