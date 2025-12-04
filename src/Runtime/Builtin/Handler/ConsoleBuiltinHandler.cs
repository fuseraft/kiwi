using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;
public static class ConsoleBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Console_Input => Input(token, args),
            TokenName.Builtin_Console_Foreground => Foreground(token, args),
            TokenName.Builtin_Console_Background => Background(token, args),
            TokenName.Builtin_Console_Clear => Clear(token, args),
            TokenName.Builtin_Console_Reset => Reset(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Clear(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.Clear, 0, args.Count);

        Console.Clear();

        return Value.Default;
    }

    private static Value Reset(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.Clear, 0, args.Count);

        Console.ResetColor();

        return Value.Default;
    }

    private static Value Input(Token token, List<Value> args)
    {
        if (args.Count != 0 && args.Count != 1)
        {
            throw new ParameterCountMismatchError(token, ConsoleBuiltin.Input);
        }

        if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectString(token, ConsoleBuiltin.Input, 0, args[0]);

            Console.Write(args[0].GetString());
        }

        var content = Console.ReadLine() ?? string.Empty;
        return Value.CreateString(content);
    }

    private static Value Foreground(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.Foreground, 1, args.Count);
        
        ParameterTypeMismatchError.ExpectInteger(token, ConsoleBuiltin.Foreground, 0, args[0]);

        Console.ForegroundColor = (ConsoleColor)args[0].GetInteger();

        return Value.Default;
    }

    private static Value Background(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.Background, 1, args.Count);

        ParameterTypeMismatchError.ExpectInteger(token, ConsoleBuiltin.Background, 0, args[0]);

        Console.BackgroundColor = (ConsoleColor)args[0].GetInteger();

        return Value.Default;
    }
}