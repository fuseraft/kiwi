namespace kiwi.Runtime.Ffi;

/// <summary>A C type that an <c>ffi</c> function can take or return, or memory can hold.</summary>
public enum FfiType
{
    Void,
    Int8,
    UInt8,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Float,
    Double,
    Bool,
    Pointer,
    String,
    Bytes,
}
