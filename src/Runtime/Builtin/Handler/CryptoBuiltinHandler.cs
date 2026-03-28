using System.Security.Cryptography;
using System.Text;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class CryptoBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Crypto_Sha1       => HashString(token, args, "SHA1"),
            TokenName.Builtin_Crypto_Sha384     => HashString(token, args, "SHA384"),
            TokenName.Builtin_Crypto_Sha512     => HashString(token, args, "SHA512"),
            TokenName.Builtin_Crypto_HmacMd5    => HmacHash(token, args, "HMACMD5"),
            TokenName.Builtin_Crypto_HmacSha256 => HmacHash(token, args, "HMACSHA256"),
            TokenName.Builtin_Crypto_HmacSha512 => HmacHash(token, args, "HMACSHA512"),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value HashString(Token token, List<Value> args, string algorithm)
    {
        ParameterCountMismatchError.Check(token, algorithm, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, algorithm, 0, args[0]);

        var input = Encoding.UTF8.GetBytes(args[0].GetString());
        byte[] hash = algorithm switch
        {
            "SHA1"   => SHA1.HashData(input),
            "SHA384" => SHA384.HashData(input),
            "SHA512" => SHA512.HashData(input),
            _ => throw new InvalidOperationException($"Unknown hash algorithm: {algorithm}")
        };

        return Value.CreateString(Convert.ToHexString(hash).ToLowerInvariant());
    }

    private static Value HmacHash(Token token, List<Value> args, string algorithm)
    {
        ParameterCountMismatchError.Check(token, algorithm, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, algorithm, 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, algorithm, 1, args[1]);

        var key   = Encoding.UTF8.GetBytes(args[0].GetString());
        var input = Encoding.UTF8.GetBytes(args[1].GetString());

        byte[] hash = algorithm switch
        {
            "HMACMD5"    => HMACMD5.HashData(key, input),
            "HMACSHA256" => HMACSHA256.HashData(key, input),
            "HMACSHA512" => HMACSHA512.HashData(key, input),
            _ => throw new InvalidOperationException($"Unknown HMAC algorithm: {algorithm}")
        };

        return Value.CreateString(Convert.ToHexString(hash).ToLowerInvariant());
    }
}
