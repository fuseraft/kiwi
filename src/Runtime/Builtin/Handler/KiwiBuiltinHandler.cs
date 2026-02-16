using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Util;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;
public static class KiwiBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Kiwi_ExecPath => ExecPath(token, args),
            TokenName.Builtin_Kiwi_Main => Main_(token, args),
            TokenName.Builtin_Kiwi_Tokenize => Tokenize(token, args),
            TokenName.Builtin_Kiwi_TypeOf => TypeOf(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }
    
    private static Value ExecPath(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, KiwiBuiltin.ExecPath, 0, args.Count);
        var execPath = Interpreter.Current?.ExecutionPath ?? string.Empty;
        if (string.IsNullOrEmpty(execPath))
        {
            return Value.EmptyString;
        }

        return Value.CreateString(FileUtil.GetAbsolutePath(token, execPath));
    }

    private static Value Main_(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, KiwiBuiltin.Main, 0, args.Count);
        return Value.CreateString(Interpreter.Current?.EntryPath ?? string.Empty);
    }

    private static Value Tokenize(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, KiwiBuiltin.Tokenize, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, KiwiBuiltin.Tokenize, 0, args[0]);
        var code = args[0].GetString();

        if (string.IsNullOrWhiteSpace(code))
        {
            return Value.CreateHashmap();
        }
        
        Dictionary<Value, Value> res = [];

        try
        {
            using Lexer lexer = new(0, code);
            var stream = lexer.GetTokenStream();
            var counter = 0;
            List<Value> tokens = [];

            while (stream.CanRead)
            {
                var t = stream.Current();
                Dictionary<Value, Value> hash = [];
                hash.Add(Value.CreateString("token"), Value.CreateInteger(++counter));
                hash.Add(Value.CreateString("text"), Value.CreateString($"{t.Text}"));
                hash.Add(Value.CreateString("span"), Value.CreateHashmap(new Dictionary<Value, Value>()
                {
                    { Value.CreateString("line"), Value.CreateInteger(t.Span.Line) },
                    { Value.CreateString("pos"), Value.CreateInteger(t.Span.Pos) }
                }));
                hash.Add(Value.CreateString("type"), Value.CreateString($"{t.Type}"));
                // hash.Add(Value.CreateString("name"), Value.CreateString($"{t.Name}"));

                tokens.Add(Value.CreateHashmap(hash));

                stream.Next();
            }

            res.Add(Value.CreateString("tokens"), Value.CreateList(tokens));
        }
        catch (Exception ex)
        {
            res.Add(Value.CreateString("error"), Value.CreateString(ex.Message));
        }

        return Value.CreateHashmap(res);
    }

    private static Value TypeOf(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, KiwiBuiltin.TypeOf, 1, args.Count);
        var target = args[0];

        return Value.CreateString(TypeRegistry.GetTypeName(target));
    }
}