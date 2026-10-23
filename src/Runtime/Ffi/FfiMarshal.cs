using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using kiwi.Typing;

namespace kiwi.Runtime.Ffi;

/// <summary>
/// Converts between Kiwi values and C values: in the 64-bit slots <see cref="FfiInvoker"/>
/// passes and returns, and in memory. Conversion errors are <see cref="FfiException"/>s
/// whose message describes the problem; the caller adds where it happened.
/// </summary>
public static unsafe class FfiMarshal
{
    /// <summary>
    /// The slot for <paramref name="value"/> as a <paramref name="type"/> argument. A string or
    /// bytes argument is copied to memory that is added to <paramref name="temporary"/>, for the
    /// caller to free once the call returns.
    /// </summary>
    public static ulong ToSlot(Value value, FfiType type, List<nint> temporary)
    {
        switch (type)
        {
            case FfiType.Float:
                return (uint)BitConverter.SingleToInt32Bits((float)Number(value, type));
            case FfiType.Double:
                return (ulong)BitConverter.DoubleToInt64Bits(Number(value, type));
            case FfiType.Bool:
                return value.IsBoolean() ? (value.GetBoolean() ? 1UL : 0UL) : throw Expected("a boolean", value);
            case FfiType.Pointer:
                return (ulong)PointerOf(value);
            case FfiType.String:
                if (value.IsNull()) return 0;
                if (!value.IsString()) throw Expected("a string", value);
                return (ulong)Temporary(System.Text.Encoding.UTF8.GetBytes(value.GetString()), temporary);
            case FfiType.Bytes:
                if (value.IsNull()) return 0;
                if (!value.IsBytes()) throw Expected("bytes", value);
                return (ulong)Temporary(value.GetBytes(), temporary);
            case FfiType.Void:
                throw new FfiException("void isn't a parameter type.");
            default:
                return (ulong)Integer(value, type);
        }
    }

    /// <summary>The Kiwi value of a <paramref name="type"/> result returned in <paramref name="slot"/>.</summary>
    public static Value FromSlot(ulong slot, FfiType type) => type switch
    {
        FfiType.Void    => Value.CreateNull(),
        FfiType.Int8    => Value.CreateInteger((sbyte)slot),
        FfiType.UInt8   => Value.CreateInteger((byte)slot),
        FfiType.Int16   => Value.CreateInteger((short)slot),
        FfiType.UInt16  => Value.CreateInteger((ushort)slot),
        FfiType.Int32   => Value.CreateInteger((int)slot),
        FfiType.UInt32  => Value.CreateInteger((uint)slot),
        FfiType.Int64 or FfiType.UInt64 => Value.CreateInteger((long)slot),
        FfiType.Bool    => Value.CreateBoolean((byte)slot != 0),
        FfiType.Float   => Value.CreateFloat(BitConverter.Int32BitsToSingle((int)(uint)slot)),
        FfiType.Double  => Value.CreateFloat(BitConverter.Int64BitsToDouble((long)slot)),
        FfiType.Pointer => Value.CreatePointer((nint)slot),
        FfiType.String  => slot == 0 ? Value.CreateNull() : Value.CreateString(Marshal.PtrToStringUTF8((nint)slot) ?? string.Empty),
        _ => throw new FfiException($"{type.Name()} isn't a return type."),
    };

    /// <summary>Reads a <paramref name="type"/> at <paramref name="address"/>. A string is read through the pointer stored there.</summary>
    public static Value Read(nint address, FfiType type)
    {
        var p = (byte*)address;
        return type switch
        {
            FfiType.Int8    => Value.CreateInteger(Unsafe.ReadUnaligned<sbyte>(p)),
            FfiType.UInt8   => Value.CreateInteger(Unsafe.ReadUnaligned<byte>(p)),
            FfiType.Int16   => Value.CreateInteger(Unsafe.ReadUnaligned<short>(p)),
            FfiType.UInt16  => Value.CreateInteger(Unsafe.ReadUnaligned<ushort>(p)),
            FfiType.Int32   => Value.CreateInteger(Unsafe.ReadUnaligned<int>(p)),
            FfiType.UInt32  => Value.CreateInteger(Unsafe.ReadUnaligned<uint>(p)),
            FfiType.Int64 or FfiType.UInt64 => Value.CreateInteger(Unsafe.ReadUnaligned<long>(p)),
            FfiType.Bool    => Value.CreateBoolean(Unsafe.ReadUnaligned<byte>(p) != 0),
            FfiType.Float   => Value.CreateFloat(Unsafe.ReadUnaligned<float>(p)),
            FfiType.Double  => Value.CreateFloat(Unsafe.ReadUnaligned<double>(p)),
            FfiType.Pointer => Value.CreatePointer(Unsafe.ReadUnaligned<nint>(p)),
            FfiType.String  => FromSlot((ulong)Unsafe.ReadUnaligned<nint>(p), FfiType.String),
            _ => throw new FfiException($"Can't read {type.Name()}; use ffi::bytes to read bytes."),
        };
    }

    /// <summary>Writes <paramref name="value"/> as a <paramref name="type"/> at <paramref name="address"/>.</summary>
    public static void Write(nint address, FfiType type, Value value)
    {
        var p = (byte*)address;
        switch (type)
        {
            case FfiType.Int8 or FfiType.UInt8:   Unsafe.WriteUnaligned(p, (byte)Integer(value, type)); break;
            case FfiType.Int16 or FfiType.UInt16: Unsafe.WriteUnaligned(p, (ushort)Integer(value, type)); break;
            case FfiType.Int32 or FfiType.UInt32: Unsafe.WriteUnaligned(p, (uint)Integer(value, type)); break;
            case FfiType.Int64 or FfiType.UInt64: Unsafe.WriteUnaligned(p, Integer(value, type)); break;
            case FfiType.Bool:
                Unsafe.WriteUnaligned(p, value.IsBoolean() ? (byte)(value.GetBoolean() ? 1 : 0) : throw Expected("a boolean", value));
                break;
            case FfiType.Float:   Unsafe.WriteUnaligned(p, (float)Number(value, type)); break;
            case FfiType.Double:  Unsafe.WriteUnaligned(p, Number(value, type)); break;
            case FfiType.Pointer: Unsafe.WriteUnaligned(p, PointerOf(value)); break;
            default:
                throw new FfiException($"Can't write {type.Name()}; use ffi::cstring for a string and ffi::write_bytes for bytes.");
        }
    }

    public static nint PointerOf(Value value)
    {
        if (value.IsNull()) return 0;
        return value.IsPointer() ? value.GetPointer() : throw Expected("a pointer", value);
    }

    private static long Integer(Value value, FfiType type)
    {
        if (!value.IsInteger()) throw Expected("an integer", value);
        var n = value.GetInteger();
        var (min, max) = type switch
        {
            FfiType.Int8   => (sbyte.MinValue, sbyte.MaxValue),
            FfiType.UInt8  => (0, byte.MaxValue),
            FfiType.Int16  => (short.MinValue, short.MaxValue),
            FfiType.UInt16 => (0, ushort.MaxValue),
            FfiType.Int32  => (int.MinValue, int.MaxValue),
            FfiType.UInt32 => (0, uint.MaxValue),
            FfiType.UInt64 => (0, long.MaxValue),
            _              => (long.MinValue, long.MaxValue),
        };
        if (n < min || n > max)
            throw new FfiException($"{n} is out of range for {type.Name()}.");
        return n;
    }

    private static double Number(Value value, FfiType type)
    {
        if (value.IsFloat()) return value.GetFloat();
        if (value.IsInteger()) return value.GetInteger();
        throw Expected("a number", value);
    }

    private static nint Temporary(byte[] data, List<nint> temporary)
    {
        // NUL-terminated, so a string can be read as a C string.
        var p = (byte*)NativeMemory.Alloc((nuint)data.Length + 1);
        temporary.Add((nint)p);
        data.CopyTo(new Span<byte>(p, data.Length));
        p[data.Length] = 0;
        return (nint)p;
    }

    private static FfiException Expected(string what, Value value) =>
        new($"expected {what}, got {TypeRegistry.GetTypeName(value)}.");
}
