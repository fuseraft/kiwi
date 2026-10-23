namespace kiwi.Runtime.Ffi;

public static class FfiTypes
{
    // `long` is 32 bits on Windows and 64 bits elsewhere, as in C.
    private static readonly FfiType CLong  = OperatingSystem.IsWindows() ? FfiType.Int32 : FfiType.Int64;
    private static readonly FfiType CULong = OperatingSystem.IsWindows() ? FfiType.UInt32 : FfiType.UInt64;

    public static bool TryParse(string name, out FfiType type)
    {
        FfiType? parsed = name switch
        {
            "void"                 => FfiType.Void,
            "int8" or "char"       => FfiType.Int8,
            "uint8" or "uchar"     => FfiType.UInt8,
            "int16" or "short"     => FfiType.Int16,
            "uint16" or "ushort"   => FfiType.UInt16,
            "int32" or "int"       => FfiType.Int32,
            "uint32" or "uint"     => FfiType.UInt32,
            "int64" or "longlong" or "ssize_t" => FfiType.Int64,
            "uint64" or "ulonglong" or "size_t" => FfiType.UInt64,
            "long"                 => CLong,
            "ulong"                => CULong,
            "float"                => FfiType.Float,
            "double"               => FfiType.Double,
            "bool"                 => FfiType.Bool,
            "pointer"              => FfiType.Pointer,
            "string"               => FfiType.String,
            "bytes"                => FfiType.Bytes,
            _                      => null,
        };
        type = parsed ?? FfiType.Void;
        return parsed != null;
    }

    /// <summary>Passed in floating-point registers rather than integer ones.</summary>
    public static bool IsFloatingPoint(this FfiType type) => type is FfiType.Float or FfiType.Double;

    /// <summary>The size in memory. A string or bytes value is a pointer.</summary>
    public static int Size(this FfiType type) => type switch
    {
        FfiType.Int8 or FfiType.UInt8 or FfiType.Bool => 1,
        FfiType.Int16 or FfiType.UInt16               => 2,
        FfiType.Int32 or FfiType.UInt32 or FfiType.Float => 4,
        FfiType.Void                                  => 0,
        _                                             => 8,
    };

    public static string Name(this FfiType type) => type switch
    {
        FfiType.Void    => "void",
        FfiType.Int8    => "int8",
        FfiType.UInt8   => "uint8",
        FfiType.Int16   => "int16",
        FfiType.UInt16  => "uint16",
        FfiType.Int32   => "int32",
        FfiType.UInt32  => "uint32",
        FfiType.Int64   => "int64",
        FfiType.UInt64  => "uint64",
        FfiType.Float   => "float",
        FfiType.Double  => "double",
        FfiType.Bool    => "bool",
        FfiType.Pointer => "pointer",
        FfiType.String  => "string",
        _               => "bytes",
    };
}
