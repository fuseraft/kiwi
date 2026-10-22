using System.Runtime.InteropServices;

namespace kiwi.Runtime.Native;

/// <summary>
/// The function table handed to native extensions. Mirrors <c>KiwiApi</c> in
/// <c>sdk/c/kiwi_ext.h</c> member for member; only ever append to it.
/// </summary>
[StructLayout(LayoutKind.Sequential)]
public unsafe struct KiwiHostApi
{
    public const uint CurrentAbiVersion = 1;

    public uint AbiVersion;
    public uint Size;

    public delegate* unmanaged[Cdecl]<nint, byte*, nint, int, int> Define;
    public delegate* unmanaged[Cdecl]<nint, byte*, void> Raise;
    public delegate* unmanaged[Cdecl]<nint, ulong, int> TypeOf;

    public delegate* unmanaged[Cdecl]<nint, ulong> NewNone;
    public delegate* unmanaged[Cdecl]<nint, long, ulong> NewInt;
    public delegate* unmanaged[Cdecl]<nint, double, ulong> NewFloat;
    public delegate* unmanaged[Cdecl]<nint, int, ulong> NewBool;
    public delegate* unmanaged[Cdecl]<nint, byte*, nuint, ulong> NewString;
    public delegate* unmanaged[Cdecl]<nint, byte*, nuint, ulong> NewBytes;
    public delegate* unmanaged[Cdecl]<nint, nint, ulong> NewPointer;

    public delegate* unmanaged[Cdecl]<nint, ulong, long> ToInt;
    public delegate* unmanaged[Cdecl]<nint, ulong, double> ToFloat;
    public delegate* unmanaged[Cdecl]<nint, ulong, int> ToBool;
    public delegate* unmanaged[Cdecl]<nint, ulong, nint> ToPointer;
    public delegate* unmanaged[Cdecl]<nint, ulong, nuint*, byte*> StringGet;
    public delegate* unmanaged[Cdecl]<nint, ulong, nuint*, byte*> BytesGet;

    public delegate* unmanaged[Cdecl]<nint, ulong> NewList;
    public delegate* unmanaged[Cdecl]<nint, ulong, long> ListLen;
    public delegate* unmanaged[Cdecl]<nint, ulong, long, ulong> ListGet;
    public delegate* unmanaged[Cdecl]<nint, ulong, ulong, int> ListPush;

    public delegate* unmanaged[Cdecl]<nint, ulong> NewHashmap;
    public delegate* unmanaged[Cdecl]<nint, ulong, ulong, ulong> HashmapGet;
    public delegate* unmanaged[Cdecl]<nint, ulong, ulong, ulong, int> HashmapSet;
    public delegate* unmanaged[Cdecl]<nint, ulong, ulong> HashmapKeys;

    public delegate* unmanaged[Cdecl]<nint, ulong, int, ulong*, ulong> Call;
}
