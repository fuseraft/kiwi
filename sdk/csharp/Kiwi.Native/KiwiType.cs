namespace Kiwi.Native;

/// <summary>
/// The type of a Kiwi value. The numbers match <c>KiwiType</c> in <c>sdk/c/kiwi_ext.h</c>.
/// </summary>
public enum KiwiType
{
    None    = 0,
    Integer = 1,
    Float   = 2,
    Boolean = 3,
    String  = 4,
    List    = 5,
    Hashmap = 6,
    Bytes   = 7,
    Pointer = 8,
    Lambda  = 9,
    Other   = 10,
}
