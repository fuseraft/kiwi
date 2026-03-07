using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;
using ValueType = kiwi.Typing.ValueType;

namespace kiwi.Runtime.Builtin.Handler;

public static class ProtobufBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Protobuf_Encode => Encode(token, args),
            TokenName.Builtin_Protobuf_Decode => Decode(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    // Proto wire types
    private const int WireVarint = 0;
    private const int Wire64Bit  = 1;
    private const int WireLength = 2;
    private const int Wire32Bit  = 5;

    private static int GetWireType(string type) => type switch
    {
        "int32" or "int64" or "uint32" or "uint64"
        or "sint32" or "sint64" or "bool" or "enum" => WireVarint,
        "fixed64" or "sfixed64" or "double"         => Wire64Bit,
        "string" or "bytes" or "message"            => WireLength,
        "float" or "fixed32" or "sfixed32"          => Wire32Bit,
        _                                           => WireLength,
    };

    // ---- Encode ----

    private static Value Encode(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ProtobufBuiltin.Encode, 2, args.Count);
        ParameterTypeMismatchError.ExpectHashmap(token, ProtobufBuiltin.Encode, 0, args[0]);
        ParameterTypeMismatchError.ExpectHashmap(token, ProtobufBuiltin.Encode, 1, args[1]);

        using var ms = new System.IO.MemoryStream();
        EncodeMessage(ms, args[0].GetHashmap(), args[1].GetHashmap());
        return Value.CreateBytes(ms.ToArray());
    }

    private static void EncodeMessage(
        System.IO.MemoryStream ms,
        Dictionary<Value, Value> schema,
        Dictionary<Value, Value> data)
    {
        // Build a name→value lookup from the data hashmap
        var dataByName = new Dictionary<string, Value>(StringComparer.Ordinal);
        foreach (var kvp in data)
        {
            dataByName[kvp.Key.GetString()] = kvp.Value;
        }

        foreach (var entry in schema)
        {
            if (!entry.Key.IsInteger() || !entry.Value.IsHashmap()) 
            {
                continue;
            }

            var fieldNum = (int)entry.Key.GetInteger();
            var fieldDef = entry.Value.GetHashmap();
            var fieldName = GetStringField(fieldDef, "name");
            var fieldType = GetStringField(fieldDef, "type");
            var repeated  = GetBoolField(fieldDef, "repeated");

            if (!dataByName.TryGetValue(fieldName, out var fieldValue)) 
            {
                continue;
            }

            if (repeated && fieldValue.IsList())
            {
                foreach (var item in fieldValue.GetList())
                {
                    EncodeField(ms, fieldNum, fieldType, fieldDef, item);
                }
            }
            else
            {
                EncodeField(ms, fieldNum, fieldType, fieldDef, fieldValue);
            }
        }
    }

    private static void EncodeField(
        System.IO.MemoryStream ms,
        int fieldNum,
        string type,
        Dictionary<Value, Value> fieldDef,
        Value value)
    {
        uint tag = (uint)((fieldNum << 3) | GetWireType(type));
        WriteVarint(ms, tag);

        switch (type)
        {
            case "int32": case "uint32": case "int64": case "uint64": case "enum":
                WriteVarint(ms, ToUInt64(value));
                break;

            case "bool":
                WriteVarint(ms, value.IsBoolean() ? (value.GetBoolean() ? 1UL : 0UL)
                                                  : (ulong)value.GetInteger());
                break;

            case "sint32":
            {
                long v = value.IsFloat() ? (long)value.GetFloat() : value.GetInteger();
                WriteVarint(ms, (ulong)((v << 1) ^ (v >> 31)));
                break;
            }

            case "sint64":
            {
                long v = value.IsFloat() ? (long)value.GetFloat() : value.GetInteger();
                WriteVarint(ms, (ulong)((v << 1) ^ (v >> 63)));
                break;
            }

            case "double":
            {
                double d = value.IsFloat() ? value.GetFloat() : value.GetInteger();
                WriteFixed64(ms, BitConverter.DoubleToUInt64Bits(d));
                break;
            }

            case "fixed64":
                WriteFixed64(ms, ToUInt64(value));
                break;

            case "sfixed64":
                WriteFixed64(ms, (ulong)ToInt64(value));
                break;

            case "float":
            {
                float f = value.IsFloat() ? (float)value.GetFloat() : (float)value.GetInteger();
                WriteFixed32(ms, BitConverter.SingleToUInt32Bits(f));
                break;
            }

            case "fixed32":
                WriteFixed32(ms, (uint)ToInt64(value));
                break;

            case "sfixed32":
                WriteFixed32(ms, (uint)(int)ToInt64(value));
                break;

            case "string":
            {
                var bytes = System.Text.Encoding.UTF8.GetBytes(value.GetString());
                WriteVarint(ms, (ulong)bytes.Length);
                ms.Write(bytes);
                break;
            }

            case "bytes":
            {
                byte[] bytes = value.IsBytes()  ? value.GetBytes()
                             : value.IsString() ? System.Text.Encoding.UTF8.GetBytes(value.GetString())
                             : [];
                WriteVarint(ms, (ulong)bytes.Length);
                ms.Write(bytes);
                break;
            }

            case "message":
            {
                if (!value.IsHashmap()) 
                {
                    break;
                }

                var nestedSchema = GetHashmapField(fieldDef, "schema");
                if (nestedSchema == null) 
                {
                    break;
                }

                using var nested = new System.IO.MemoryStream();
                EncodeMessage(nested, nestedSchema, value.GetHashmap());
                var encoded = nested.ToArray();
                WriteVarint(ms, (ulong)encoded.Length);
                ms.Write(encoded);
                break;
            }
        }
    }

    // ---- Decode ----

    private static Value Decode(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ProtobufBuiltin.Decode, 2, args.Count);
        ParameterTypeMismatchError.ExpectHashmap(token, ProtobufBuiltin.Decode, 0, args[0]);

        byte[] data;
        if (args[1].IsBytes())
        {
            data = args[1].GetBytes();
        }
        else if (args[1].IsString())
        {
            data = System.Text.Encoding.UTF8.GetBytes(args[1].GetString());
        }
        else
        {
            throw new ParameterTypeMismatchError(token, ProtobufBuiltin.Decode, 1, ValueType.Bytes, args[1].Type);
        }

        return Value.CreateHashmap(DecodeMessage(data, 0, data.Length, args[0].GetHashmap()));
    }

    private static Dictionary<Value, Value> DecodeMessage(
        byte[] data,
        int offset,
        int end,
        Dictionary<Value, Value> schema)
    {
        // Build field-number → field-def lookup
        var byNum = new Dictionary<int, Dictionary<Value, Value>>();
        foreach (var kvp in schema)
        {
            if (kvp.Key.IsInteger() && kvp.Value.IsHashmap())
            {
                byNum[(int)kvp.Key.GetInteger()] = kvp.Value.GetHashmap();
            }
        }

        var result = new Dictionary<Value, Value>();

        // Pre-seed repeated fields as empty lists
        foreach (var kvp in byNum)
        {
            if (GetBoolField(kvp.Value, "repeated"))
            {
                result[Value.CreateString(GetStringField(kvp.Value, "name"))] = Value.CreateList([]);
            }
        }

        var pos = offset;
        while (pos < end)
        {
            var tag      = ReadVarint(data, ref pos);
            var fieldNum = (int)(tag >> 3);
            var wireType = (int)(tag & 7);

            if (!byNum.TryGetValue(fieldNum, out var fieldDef))
            {
                SkipField(data, ref pos, wireType);
                continue;
            }

            var fieldName = GetStringField(fieldDef, "name");
            var fieldType = GetStringField(fieldDef, "type");
            var repeated  = GetBoolField(fieldDef, "repeated");
            var key       = Value.CreateString(fieldName);
            var decoded   = DecodeFieldValue(data, ref pos, wireType, fieldType, fieldDef);

            if (repeated)
            {
                if (!result.TryGetValue(key, out var list))
                {
                    list = Value.CreateList([]);
                    result[key] = list;
                }
                list.GetList().Add(decoded);
            }
            else
            {
                result[key] = decoded;
            }
        }

        return result;
    }

    private static Value DecodeFieldValue(
        byte[] data,
        ref int pos,
        int wireType,
        string fieldType,
        Dictionary<Value, Value> fieldDef)
    {
        switch (fieldType)
        {
            case "int32": case "uint32": case "int64": case "uint64": case "enum":
                return Value.CreateInteger((long)ReadVarint(data, ref pos));

            case "sint32": case "sint64":
            {
                var raw = ReadVarint(data, ref pos);
                return Value.CreateInteger((long)((raw >> 1) ^ (ulong)(-(long)(raw & 1))));
            }

            case "bool":
                return ReadVarint(data, ref pos) != 0 ? Value.True : Value.False;

            case "double":
            {
                var bytes = ReadFixed(data, ref pos, 8);
                if (!BitConverter.IsLittleEndian) Array.Reverse(bytes);
                return Value.CreateFloat(BitConverter.ToDouble(bytes));
            }

            case "fixed64":
            {
                var bytes = ReadFixed(data, ref pos, 8);
                if (!BitConverter.IsLittleEndian) Array.Reverse(bytes);
                return Value.CreateInteger((long)BitConverter.ToUInt64(bytes));
            }

            case "sfixed64":
            {
                var bytes = ReadFixed(data, ref pos, 8);
                if (!BitConverter.IsLittleEndian) Array.Reverse(bytes);
                return Value.CreateInteger(BitConverter.ToInt64(bytes));
            }

            case "float":
            {
                var bytes = ReadFixed(data, ref pos, 4);
                if (!BitConverter.IsLittleEndian) Array.Reverse(bytes);
                return Value.CreateFloat(BitConverter.ToSingle(bytes));
            }

            case "fixed32":
            {
                var bytes = ReadFixed(data, ref pos, 4);
                if (!BitConverter.IsLittleEndian) Array.Reverse(bytes);
                return Value.CreateInteger(BitConverter.ToUInt32(bytes));
            }

            case "sfixed32":
            {
                var bytes = ReadFixed(data, ref pos, 4);
                if (!BitConverter.IsLittleEndian) Array.Reverse(bytes);
                return Value.CreateInteger(BitConverter.ToInt32(bytes));
            }

            case "string":
            {
                var len   = (int)ReadVarint(data, ref pos);
                var bytes = ReadFixed(data, ref pos, len);
                return Value.CreateString(System.Text.Encoding.UTF8.GetString(bytes));
            }

            case "bytes":
            {
                var len   = (int)ReadVarint(data, ref pos);
                return Value.CreateBytes(ReadFixed(data, ref pos, len));
            }

            case "message":
            {
                var len         = (int)ReadVarint(data, ref pos);
                var msgEnd      = pos + len;
                var nestedSchema = GetHashmapField(fieldDef, "schema");
                Value result    = nestedSchema != null
                    ? Value.CreateHashmap(DecodeMessage(data, pos, msgEnd, nestedSchema))
                    : Value.CreateBytes(data[pos..msgEnd]);
                pos = msgEnd;
                return result;
            }

            default:
                return wireType switch
                {
                    WireVarint => Value.CreateInteger((long)ReadVarint(data, ref pos)),
                    Wire64Bit  => Value.CreateInteger((long)BitConverter.ToUInt64(ReadFixed(data, ref pos, 8))),
                    Wire32Bit  => Value.CreateInteger(BitConverter.ToUInt32(ReadFixed(data, ref pos, 4))),
                    _          => // WireLength
                    (
                        Value.CreateBytes(ReadFixed(data, ref pos, (int)ReadVarint(data, ref pos)))
                    ),
                };
        }
    }

    // ---- Wire I/O helpers ----

    private static void WriteVarint(System.IO.MemoryStream ms, ulong value)
    {
        while (value >= 0x80)
        {
            ms.WriteByte((byte)(value | 0x80));
            value >>= 7;
        }
        ms.WriteByte((byte)value);
    }

    private static void WriteFixed64(System.IO.MemoryStream ms, ulong value)
    {
        var bytes = BitConverter.GetBytes(value);
        if (!BitConverter.IsLittleEndian)
        {
            Array.Reverse(bytes);
        }

        ms.Write(bytes);
    }

    private static void WriteFixed32(System.IO.MemoryStream ms, uint value)
    {
        var bytes = BitConverter.GetBytes(value);
        if (!BitConverter.IsLittleEndian)
        {
            Array.Reverse(bytes);
        }
        ms.Write(bytes);
    }

    private static ulong ReadVarint(byte[] data, ref int pos)
    {
        ulong result = 0;
        var shift = 0;
        while (pos < data.Length)
        {
            var b = data[pos++];
            result |= (ulong)(b & 0x7F) << shift;
            if ((b & 0x80) == 0) 
            {
                break;
            }
            shift += 7;
        }
        return result;
    }

    private static byte[] ReadFixed(byte[] data, ref int pos, int count)
    {
        var bytes = data[pos..(pos + count)];
        pos += count;
        return bytes;
    }

    private static void SkipField(byte[] data, ref int pos, int wireType)
    {
        switch (wireType)
        {
            case WireVarint: ReadVarint(data, ref pos); break;
            case Wire64Bit:  pos += 8; break;
            case Wire32Bit:  pos += 4; break;
            default:         pos += (int)ReadVarint(data, ref pos); break; // WireLength
        }
    }

    // ---- Schema/value helpers ----

    private static ulong ToUInt64(Value v)
        => v.IsFloat() ? (ulong)(long)v.GetFloat() : (ulong)v.GetInteger();

    private static long ToInt64(Value v)
        => v.IsFloat() ? (long)v.GetFloat() : v.GetInteger();

    private static string GetStringField(Dictionary<Value, Value> map, string key)
        => map.TryGetValue(Value.CreateString(key), out var v) ? v.GetString() : "";

    private static bool GetBoolField(Dictionary<Value, Value> map, string key)
        => map.TryGetValue(Value.CreateString(key), out var v) && v.IsBoolean() && v.GetBoolean();

    private static Dictionary<Value, Value>? GetHashmapField(Dictionary<Value, Value> map, string key)
        => map.TryGetValue(Value.CreateString(key), out var v) && v.IsHashmap() ? v.GetHashmap() : null;
}
