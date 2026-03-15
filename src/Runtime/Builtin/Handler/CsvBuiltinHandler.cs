using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Util;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class CsvBuiltinHandler
{
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

        var rows = ParseCsv(input, delim);
        return Value.CreateList(BuildMaps(rows, headers, schema));
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
        var rows    = ParseCsv(content, delim);
        return Value.CreateList(BuildMaps(rows, headers, schema));
    }

    private static List<Value> BuildMaps(List<Value> rows, List<Value>? customHeaders, List<Value>? schema)
    {
        if (rows.Count == 0)
            return [];

        List<Value> headers;
        List<Value> dataRows;

        if (customHeaders != null)
        {
            headers  = customHeaders;
            dataRows = rows;
        }
        else
        {
            headers  = rows[0].GetList();
            dataRows = rows.Count > 1 ? rows.GetRange(1, rows.Count - 1) : [];
        }

        var colCount = headers.Count;
        var result   = new List<Value>(dataRows.Count);

        foreach (var rowVal in dataRows)
        {
            var row    = rowVal.GetList();
            var record = new Dictionary<Value, Value>(colCount);

            for (var i = 0; i < colCount; i++)
            {
                var key = headers[i];
                var val = i < row.Count ? row[i] : Value.Default;

                if (schema != null && i < schema.Count)
                {
                    val = CoerceValue(val, schema[i].GetString());
                }

                record[key] = val;
            }

            result.Add(Value.CreateHashmap(record));
        }

        return result;
    }

    private static Value CoerceValue(Value val, string schemaType)
    {
        var t = schemaType.ToLowerInvariant();

        if (t == "string")
            return val;

        var raw = val.IsString() ? val.GetString() : val.GetString();

        if (raw.Length == 0 && t != "null" && t != "none")
            return Value.Default;

        return t switch
        {
            "integer" or "int" =>
                long.TryParse(raw, out var i)
                    ? Value.CreateInteger(i)
                    : Value.Default,

            "float" or "double" or "number" =>
                double.TryParse(raw, System.Globalization.NumberStyles.Float,
                    System.Globalization.CultureInfo.InvariantCulture, out var d)
                    ? Value.CreateFloat(d)
                    : Value.Default,

            "boolean" or "bool" =>
                Value.CreateBoolean(raw is "true" or "1" or "yes"),

            "date" =>
                DateTime.TryParse(raw, out var dt)
                    ? Value.CreateDate(dt)
                    : Value.Default,

            "null" or "none" => Value.Default,

            _ => val,
        };
    }

    private static List<Value> ParseCsv(string input, char delim)
    {
        var rows = new List<Value>();
        var currentRow = new List<Value>();
        var currentField = new System.Text.StringBuilder();
        var inQuote = false;
        var len = input.Length;

        for (var i = 0; i < len; i++)
        {
            var c = input[i];

            if (inQuote)
            {
                if (c == '"')
                {
                    if (i + 1 < len && input[i + 1] == '"')
                    {
                        // escaped double-quote
                        currentField.Append('"');
                        i++;
                    }
                    else
                    {
                        // end of quoted field
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
                        i++; // skip LF after CR
                    }
                    currentRow.Add(Value.CreateString(currentField.ToString()));
                    rows.Add(Value.CreateList(currentRow));
                    currentRow = new List<Value>();
                    currentField.Clear();
                }
                else
                {
                    currentField.Append(c);
                }
            }
        }

        // add last field and row (for files without a trailing newline)
        if (currentField.Length > 0 || currentRow.Count > 0)
        {
            currentRow.Add(Value.CreateString(currentField.ToString()));
            rows.Add(Value.CreateList(currentRow));
        }

        return rows;
    }
}
