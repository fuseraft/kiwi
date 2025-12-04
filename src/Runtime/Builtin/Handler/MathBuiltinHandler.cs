using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class MathBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Math_Abs => Abs(token, args),
            TokenName.Builtin_Math_Ceil => Ceil(token, args),
            TokenName.Builtin_Math_Floor => Floor(token, args),
            TokenName.Builtin_Math_Random => Random(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Abs(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MathBuiltin.Abs, 1, args.Count);

        ParameterTypeMismatchError.ExpectNumber(token, MathBuiltin.Abs, 0, args[0]);

        if (args[0].IsInteger())
        {
            return Value.CreateInteger(Math.Abs(args[0].GetInteger()));
        }
        else
        {
            return Value.CreateFloat(Math.Abs(args[0].GetFloat()));
        }
    }

    private static Value Ceil(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MathBuiltin.Ceil, 1, args.Count);

        ParameterTypeMismatchError.ExpectFloat(token, MathBuiltin.Ceil, 0, args[0]);

        return Value.CreateFloat(Math.Ceiling(args[0].GetFloat()));
    }

    private static Value Floor(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MathBuiltin.Floor, 1, args.Count);

        ParameterTypeMismatchError.ExpectFloat(token, MathBuiltin.Floor, 0, args[0]);

        return Value.CreateFloat(Math.Floor(args[0].GetFloat()));
    }

    private static Value Random(Token token, List<Value> args)
    {
        if (args.Count != 0 && args.Count != 2)
        {
            throw new ParameterCountMismatchError(token, MathBuiltin.Random);
        }
        
        if (args.Count == 2)
        {
            if (args[0].IsInteger() && args[1].IsInteger())
            {
                List<long> ints = [args[0].GetInteger(), args[1].GetInteger()];
                var rand = System.Random.Shared.NextInt64(ints.Min(), ints.Max() + 1);
                return Value.CreateInteger(rand);
            }
            else if (args[0].IsString() && args[1].IsInteger())
            {
                var allowedChars = args[0].GetString();
                var length = (int)args[1].GetInteger();
                var sb = new System.Text.StringBuilder(length);
                for (int i = 0; i < length; i++)
                {
                    var index = System.Random.Shared.Next(allowedChars.Length);
                    sb.Append(allowedChars[index]);
                }
                return Value.CreateString(sb.ToString());
            }
            else if (args[0].IsList() && args[1].IsInteger())
            {
                var allowedValues = args[0].GetList();
                var length = (int)args[1].GetInteger();
                List<Value> list = [];
                for (int i = 0; i < length; i++)
                {
                    var index = System.Random.Shared.Next(allowedValues.Count);
                    list.Add(allowedValues[index]);
                }
                return Value.CreateList(list);
            }
        }

        return Value.CreateFloat(System.Random.Shared.NextDouble());
    }
}