using System.Globalization;
using System.Text.Json;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class JsonBuiltinHandler
{
    private enum SchemaKind { String, Integer, Float, Boolean, Null }

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Json_Parse     => Parse(token, args),
            TokenName.Builtin_Json_Stringify => Stringify(token, args),
            TokenName.Builtin_Json_ParseMaps => ParseMaps(token, args),
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

    // __json_parse_maps__(json_string, schema_or_null)
    // Parses JSON, wraps a top-level object in a list, and applies named-field schema coercion.
    private static Value ParseMaps(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, JsonBuiltin.ParseMaps, 1, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, JsonBuiltin.ParseMaps, 0, args[0]);

        var schema = args.Count > 1 && args[1].IsHashmap() ? args[1].GetHashmap() : null;
        var cache  = schema != null ? BuildJsonSchemaCache(schema) : null;

        try
        {
            using var doc = JsonDocument.Parse(args[0].GetString(),
                new JsonDocumentOptions { AllowTrailingCommas = true, CommentHandling = JsonCommentHandling.Skip });

            List<Value> rows;
            if (doc.RootElement.ValueKind == JsonValueKind.Array)
            {
                rows = new List<Value>(doc.RootElement.GetArrayLength());
                foreach (var el in doc.RootElement.EnumerateArray())
                    rows.Add(ConvertElement(el));
            }
            else
            {
                rows = [ConvertElement(doc.RootElement)];
            }

            if (cache != null)
            {
                for (var i = 0; i < rows.Count; i++)
                {
                    if (rows[i].IsHashmap())
                        rows[i] = ApplyJsonSchema(rows[i].GetHashmap(), cache);
                }
            }

            return Value.CreateList(rows);
        }
        catch (JsonException ex)
        {
            throw new KiwiError(token, "JsonError", ex.Message);
        }
    }

    private static Value ApplyJsonSchema(Dictionary<Value, Value> row, Dictionary<string, SchemaKind> cache)
    {
        var result = new Dictionary<Value, Value>(row.Count);
        foreach (var kvp in row)
        {
            result[kvp.Key] = kvp.Key.IsString() && cache.TryGetValue(kvp.Key.GetString(), out var kind)
                ? CoerceValue(kvp.Value, kind)
                : kvp.Value;
        }
        return Value.CreateHashmap(result);
    }

    private static Dictionary<string, SchemaKind> BuildJsonSchemaCache(Dictionary<Value, Value> schema)
    {
        var cache = new Dictionary<string, SchemaKind>(schema.Count, StringComparer.OrdinalIgnoreCase);
        foreach (var kvp in schema)
        {
            if (!kvp.Key.IsString()) continue;
            cache[kvp.Key.GetString()] = (kvp.Value.IsString() ? kvp.Value.GetString() : "string")
                .ToLowerInvariant() switch
            {
                "integer" or "int"              => SchemaKind.Integer,
                "float" or "double" or "number" => SchemaKind.Float,
                "boolean" or "bool"             => SchemaKind.Boolean,
                "null" or "none"                => SchemaKind.Null,
                _                               => SchemaKind.String,
            };
        }
        return cache;
    }

    private static Value CoerceValue(Value v, SchemaKind kind) => kind switch
    {
        SchemaKind.Integer => v.IsInteger() ? v
            : v.IsFloat()  ? Value.CreateInteger((long)v.GetFloat())
            : v.IsString() && long.TryParse(v.GetString(), out var i) ? Value.CreateInteger(i)
            : Value.Default,
        SchemaKind.Float   => v.IsFloat()   ? v
            : v.IsInteger() ? Value.CreateFloat((double)v.GetInteger())
            : v.IsString()  && double.TryParse(v.GetString(), NumberStyles.Float,
                CultureInfo.InvariantCulture, out var d) ? Value.CreateFloat(d)
            : Value.Default,
        SchemaKind.Boolean => v.IsBoolean() ? v
            : Value.CreateBoolean(v.IsString() && v.GetString() is "true" or "1" or "yes"),
        SchemaKind.String  => v.IsString()  ? v : Value.CreateString(Serializer.Serialize(v)),
        SchemaKind.Null    => Value.Default,
        _                  => v,
    };

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
