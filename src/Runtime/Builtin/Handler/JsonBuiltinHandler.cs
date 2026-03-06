using System.Text.Json;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class JsonBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Json_Parse     => Parse(token, args),
            TokenName.Builtin_Json_Stringify => Stringify(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Parse(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, JsonBuiltin.Parse, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, JsonBuiltin.Parse, 0, args[0]);

        try
        {
            using var doc = JsonDocument.Parse(args[0].GetString(),
                new JsonDocumentOptions { AllowTrailingCommas = true, CommentHandling = JsonCommentHandling.Skip });
            return ConvertElement(doc.RootElement);
        }
        catch (JsonException ex)
        {
            throw new KiwiError(token, "JsonError", ex.Message);
        }
    }

    private static Value ConvertElement(JsonElement el) => el.ValueKind switch
    {
        JsonValueKind.Null    => Value.CreateNull(),
        JsonValueKind.True    => Value.True,
        JsonValueKind.False   => Value.False,
        JsonValueKind.Number  => ConvertNumber(el),
        JsonValueKind.String  => Value.CreateString(el.GetString() ?? string.Empty),
        JsonValueKind.Array   => ConvertArray(el),
        JsonValueKind.Object  => ConvertObject(el),
        _                     => Value.Default,
    };

    private static Value ConvertNumber(JsonElement el)
    {
        if (el.TryGetInt64(out long lval))
        {
            return Value.CreateInteger(lval);
        }
        return Value.CreateFloat(el.GetDouble());
    }

    private static Value ConvertArray(JsonElement el)
    {
        var list = new List<Value>(el.GetArrayLength());
        foreach (var item in el.EnumerateArray())
        {
            list.Add(ConvertElement(item));
        }
        return Value.CreateList(list);
    }

    private static Value ConvertObject(JsonElement el)
    {
        var map = new Dictionary<Value, Value>();
        foreach (var prop in el.EnumerateObject())
        {
            map[Value.CreateString(prop.Name)] = ConvertElement(prop.Value);
        }
        return Value.CreateHashmap(map);
    }

    private static Value Stringify(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, JsonBuiltin.Stringify, 1, 2, args.Count);

        var pretty = args.Count > 1 && args[1].IsBoolean() && args[1].GetBoolean();

        using var ms = new System.IO.MemoryStream();
        using var writer = new Utf8JsonWriter(ms, new JsonWriterOptions { Indented = pretty });
        WriteValue(writer, args[0]);
        writer.Flush();
        return Value.CreateString(System.Text.Encoding.UTF8.GetString(ms.ToArray()));
    }

    private static void WriteValue(Utf8JsonWriter w, Value v)
    {
        if (v.IsNull())
            w.WriteNullValue();
        else if (v.IsBoolean())
            w.WriteBooleanValue(v.GetBoolean());
        else if (v.IsInteger())
            w.WriteNumberValue(v.GetInteger());
        else if (v.IsFloat())
            w.WriteNumberValue(v.GetFloat());
        else if (v.IsString())
            w.WriteStringValue(v.GetString());
        else if (v.IsList())
        {
            w.WriteStartArray();
            foreach (var item in v.GetList())
                WriteValue(w, item);
            w.WriteEndArray();
        }
        else if (v.IsHashmap())
        {
            w.WriteStartObject();
            foreach (var kvp in v.GetHashmap())
            {
                w.WritePropertyName(Serializer.Serialize(kvp.Key));
                WriteValue(w, kvp.Value);
            }
            w.WriteEndObject();
        }
        else if (v.IsObject())
        {
            w.WriteStartObject();
            foreach (var field in v.GetObject().InstanceVariables)
            {
                // Strip leading @ from instance variable names
                var key = field.Key.StartsWith('@') ? field.Key[1..] : field.Key;
                w.WritePropertyName(key);
                WriteValue(w, field.Value);
            }
            w.WriteEndObject();
        }
        else
        {
            // Fallback: serialize as string
            w.WriteStringValue(Serializer.Serialize(v));
        }
    }
}
