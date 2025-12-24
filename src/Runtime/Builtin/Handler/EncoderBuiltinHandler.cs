using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class EncoderBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Encoder_Base64Decode => Base64Decode(token, args),
            TokenName.Builtin_Encoder_Base64Encode => Base64Encode(token, args),
            TokenName.Builtin_Encoder_UrlDecode => UrlDecode(token, args),
            TokenName.Builtin_Encoder_UrlEncode => UrlEncode(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Base64Decode(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EncoderBuiltin.Base64Decode, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, EncoderBuiltin.Base64Decode, 0, args[0]);

        var base64 = args[0].GetString();

        if (string.IsNullOrEmpty(base64))
        {
            return Value.CreateList([]);
        }

        var bytes = Convert.FromBase64String(base64).Select(x => Value.CreateInteger(x)).ToList();
        return Value.CreateList(bytes);
    }

    private static Value Base64Encode(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EncoderBuiltin.Base64Encode, 1, args.Count);        
        ParameterTypeMismatchError.ExpectList(token, EncoderBuiltin.Base64Encode, 0, args[0]);

        if (args[0].IsList())
        {
            List<byte> resultBytes = [];

            foreach (var item in args[0].GetList())
            {
                TypeError.ExpectInteger(token, item);
                TypeError.ByteCheck(token, item.GetInteger());

                resultBytes.Add((byte)item.GetInteger());
            }

            var res = Convert.ToBase64String(resultBytes.ToArray());
            return Value.CreateString(res);            
        }
        else if (args[0].IsBytes())
        {
            var res = Convert.ToBase64String(args[0].GetBytes());
            return Value.CreateString(res);
        }

        throw new InvalidOperationError(token, "Expected a list or bytes.");
    }

    private static Value UrlDecode(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EncoderBuiltin.UrlDecode, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, EncoderBuiltin.UrlDecode, 0, args[0]);

        var res = System.Web.HttpUtility.UrlDecode(args[0].GetString()) ?? string.Empty;
        return Value.CreateString(res);
    }

    private static Value UrlEncode(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EncoderBuiltin.UrlEncode, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, EncoderBuiltin.UrlEncode, 0, args[0]);

        var res = System.Web.HttpUtility.UrlEncode(args[0].GetString()) ?? string.Empty;
        return Value.CreateString(res);
    }
}