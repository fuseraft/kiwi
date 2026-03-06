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
            TokenName.Builtin_Csv_Parse     => Parse(token, args),
            TokenName.Builtin_Csv_ParseFile => ParseFile(token, args),
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
