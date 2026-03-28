using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Util;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class CsvBuiltinHandler
{
    // Pre-computed schema type tag — built once per parse call, eliminates per-field ToLowerInvariant().
    private enum SchemaKind { String, NullableString, Integer, Float, Boolean, Date, Null }

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Csv_Parse         => Parse(token, args),
            TokenName.Builtin_Csv_ParseFile     => ParseFile(token, args),
            TokenName.Builtin_Csv_ParseMaps     => ParseMaps(token, args),
            TokenName.Builtin_Csv_ParseFileMaps => ParseFileMaps(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Parse(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, CsvBuiltin.Parse, 1, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, CsvBuiltin.Parse, 0, args[0]);

        var input = args[0].GetString();
        var delimiter = args.Count > 1 ? args[1].GetString() : ",";
        var delim = delimiter.Length > 0 ? delimiter[0] : ',';

        return Value.CreateList(ParseCsv(input, delim));
    }

    private static Value ParseFile(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, CsvBuiltin.ParseFile, 1, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, CsvBuiltin.ParseFile, 0, args[0]);

        var path = args[0].GetString();
        var delimiter = args.Count > 1 ? args[1].GetString() : ",";
        var delim = delimiter.Length > 0 ? delimiter[0] : ',';

        var content = FileUtil.ReadFile(token, path);
        return Value.CreateList(ParseCsv(content, delim));
    }

    // __csv_parse_maps__(input, delimiter, headers_or_null, schema_or_null)
    private static Value ParseMaps(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, CsvBuiltin.ParseMaps, 1, 4, args.Count);
        ParameterTypeMismatchError.ExpectString(token, CsvBuiltin.ParseMaps, 0, args[0]);

        var input     = args[0].GetString();
        var delimiter = args.Count > 1 ? args[1].GetString() : ",";
        var delim     = delimiter.Length > 0 ? delimiter[0] : ',';
        var headers   = args.Count > 2 && args[2].IsList() ? args[2].GetList() : null;
        var schema    = args.Count > 3 && args[3].IsList() ? args[3].GetList() : null;

        return Value.CreateList(BuildMapsFromStrings(ParseCsvToStringArrays(input, delim), headers, schema));
    }

    // __csv_parsefile_maps__(path, delimiter, headers_or_null, schema_or_null)
    private static Value ParseFileMaps(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, CsvBuiltin.ParseFileMaps, 1, 4, args.Count);
        ParameterTypeMismatchError.ExpectString(token, CsvBuiltin.ParseFileMaps, 0, args[0]);

        var path      = args[0].GetString();
        var delimiter = args.Count > 1 ? args[1].GetString() : ",";
        var delim     = delimiter.Length > 0 ? delimiter[0] : ',';
        var headers   = args.Count > 2 && args[2].IsList() ? args[2].GetList() : null;
        var schema    = args.Count > 3 && args[3].IsList() ? args[3].GetList() : null;

        var content = FileUtil.ReadFile(token, path);
        return Value.CreateList(BuildMapsFromStrings(ParseCsvToStringArrays(content, delim), headers, schema));
    }

    // ---------------------------------------------------------------------------
    // Optimised map-building path
    //
    // Differences from the old BuildMaps:
    //   1. Receives raw string[] rows — no intermediate Value per field.
    //   2. Schema types are pre-cached as SchemaKind (one ToLowerInvariant per
    //      column, not one per field × row).
    //   3. Hashmap construction is parallelised across all available CPU cores.
    // ---------------------------------------------------------------------------
    private static List<Value> BuildMapsFromStrings(
        List<string[]> rawRows,
        List<Value>?   customHeaders,
        List<Value>?   schema)
    {
        if (rawRows.Count == 0)
            return [];

        List<Value> headers;
        int         dataStart;

        if (customHeaders != null)
        {
            headers   = customHeaders;
            dataStart = 0;
        }
        else
        {
            headers   = [.. rawRows[0].Select(Value.CreateString)];
            dataStart = 1;
        }

        var colCount    = headers.Count;
        var schemaCache = schema != null ? BuildSchemaCache(schema) : null;
        var dataCount   = rawRows.Count - dataStart;
        var result      = new Value[dataCount];

        Parallel.For(0, dataCount, i =>
        {
            var fields = rawRows[dataStart + i];
            var record = new Dictionary<Value, Value>(colCount);

            for (var j = 0; j < colCount; j++)
            {
                var raw = j < fields.Length ? fields[j] : "";
                var val = schemaCache != null && j < schemaCache.Length
                    ? CoerceRaw(raw, schemaCache[j])
                    : Value.CreateString(raw);
                record[headers[j]] = val;
            }

            result[i] = Value.CreateHashmap(record);
        });

        return [.. result];
    }

    // Build the schema-kind cache once per parse call (28 entries for LAPD data).
    private static SchemaKind[] BuildSchemaCache(List<Value> schema)
    {
        var cache = new SchemaKind[schema.Count];
        for (var i = 0; i < schema.Count; i++)
        {
            cache[i] = schema[i].GetString().ToLowerInvariant() switch
            {
                "string"                        => SchemaKind.String,
                "string?" or "nullable_string"  => SchemaKind.NullableString,
                "integer" or "int"              => SchemaKind.Integer,
                "float" or "double" or "number" => SchemaKind.Float,
                "boolean" or "bool"             => SchemaKind.Boolean,
                "date"                          => SchemaKind.Date,
                "null" or "none"                => SchemaKind.Null,
                _                               => SchemaKind.String,
            };
        }
        return cache;
    }

    // Fast coercion from raw string using the pre-cached schema kind.
    private static Value CoerceRaw(string raw, SchemaKind kind)
    {
        return kind switch
        {
            SchemaKind.String         => Value.CreateString(raw),
            SchemaKind.NullableString => raw.Length == 0 ? Value.Default : Value.CreateString(raw),
            SchemaKind.Integer        => raw.Length == 0 ? Value.Default
                                            : long.TryParse(raw, out var i)
                                                ? Value.CreateInteger(i) : Value.Default,
            SchemaKind.Float          => raw.Length == 0 ? Value.Default
                                            : double.TryParse(raw,
                                                System.Globalization.NumberStyles.Float,
                                                System.Globalization.CultureInfo.InvariantCulture,
                                                out var d)
                                                    ? Value.CreateFloat(d) : Value.Default,
            SchemaKind.Boolean        => Value.CreateBoolean(raw is "true" or "1" or "yes"),
            SchemaKind.Date           => raw.Length == 0 ? Value.Default
                                            : DateTime.TryParse(raw, out var dt)
                                                ? Value.CreateDate(dt) : Value.Default,
            SchemaKind.Null           => Value.Default,
            _                         => Value.CreateString(raw),
        };
    }

    // ---------------------------------------------------------------------------
    // CSV parser — returns raw string[] per row (no Value allocation).
    // Pre-allocates the row list based on estimated row count to avoid
    // repeated list growth on large files.
    // ---------------------------------------------------------------------------
    private static List<string[]> ParseCsvToStringArrays(string input, char delim)
    {
        var estimatedRows = Math.Max(16, input.Length / 200);
        var rows          = new List<string[]>(estimatedRows);
        var currentRow    = new List<string>(32);
        var field         = new System.Text.StringBuilder(64);
        var inQuote       = false;
        var len           = input.Length;

        for (var i = 0; i < len; i++)
        {
            var c = input[i];

            if (inQuote)
            {
                if (c == '"')
                {
                    if (i + 1 < len && input[i + 1] == '"')
                    {
                        field.Append('"');
                        i++;
                    }
                    else
                    {
                        inQuote = false;
                    }
                }
                else
                {
                    field.Append(c);
                }
            }
            else
            {
                if (c == '"')
                {
                    inQuote = true;
                }
                else if (c == delim)
                {
                    currentRow.Add(field.ToString());
                    field.Clear();
                }
                else if (c == '\r' || c == '\n')
                {
                    if (c == '\r' && i + 1 < len && input[i + 1] == '\n')
                        i++;

                    currentRow.Add(field.ToString());
                    field.Clear();
                    rows.Add([.. currentRow]);
                    currentRow.Clear();
                }
                else
                {
                    field.Append(c);
                }
            }
        }

        // Handle file with no trailing newline.
        if (field.Length > 0 || currentRow.Count > 0)
        {
            currentRow.Add(field.ToString());
            rows.Add([.. currentRow]);
        }

        return rows;
    }

    // ---------------------------------------------------------------------------
    // Legacy path — still used by csv::parse and csv::parse_file (no schema/maps).
    // ---------------------------------------------------------------------------
    private static List<Value> ParseCsv(string input, char delim)
    {
        var rows         = new List<Value>();
        var currentRow   = new List<Value>();
        var currentField = new System.Text.StringBuilder();
        var inQuote      = false;
        var len          = input.Length;

        for (var i = 0; i < len; i++)
        {
            var c = input[i];

            if (inQuote)
            {
                if (c == '"')
                {
                    if (i + 1 < len && input[i + 1] == '"')
                    {
                        currentField.Append('"');
                        i++;
                    }
                    else
                    {
                        inQuote = false;
                    }
                }
                else
                {
                    currentField.Append(c);
                }
            }
            else
            {
                if (c == '"')
                {
                    inQuote = true;
                }
                else if (c == delim)
                {
                    currentRow.Add(Value.CreateString(currentField.ToString()));
                    currentField.Clear();
                }
                else if (c == '\r' || c == '\n')
                {
                    if (c == '\r' && i + 1 < len && input[i + 1] == '\n')
                    {
                        i++;
                    }
                    currentRow.Add(Value.CreateString(currentField.ToString()));
                    rows.Add(Value.CreateList(currentRow));
                    currentRow   = new List<Value>();
                    currentField.Clear();
                }
                else
                {
                    currentField.Append(c);
                }
            }
        }

        if (currentField.Length > 0 || currentRow.Count > 0)
        {
            currentRow.Add(Value.CreateString(currentField.ToString()));
            rows.Add(Value.CreateList(currentRow));
        }

        return rows;
    }
}
