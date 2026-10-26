using System.Security.Cryptography;
using System.Text;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Settings;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class CryptoBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Crypto_Md5        => Hash(token, args, "MD5"),
            TokenName.Builtin_Crypto_Sha1       => Hash(token, args, "SHA1"),
            TokenName.Builtin_Crypto_Sha256     => Hash(token, args, "SHA256"),
            TokenName.Builtin_Crypto_Sha384     => Hash(token, args, "SHA384"),
            TokenName.Builtin_Crypto_Sha512     => Hash(token, args, "SHA512"),
            TokenName.Builtin_Crypto_FileHash   => FileHash(token, args),
            TokenName.Builtin_Crypto_HmacMd5    => HmacHash(token, args, "HMACMD5"),
            TokenName.Builtin_Crypto_HmacSha256 => HmacHash(token, args, "HMACSHA256"),
            TokenName.Builtin_Crypto_HmacSha512 => HmacHash(token, args, "HMACSHA512"),
            TokenName.Builtin_Crypto_RandomBytes => RandomBytes(token, args),
            TokenName.Builtin_Crypto_Uuid       => Uuid(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    /// <summary>Bytes from the operating system's cryptographically secure generator.</summary>
    private static Value RandomBytes(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CryptoBuiltin.RandomBytes, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, CryptoBuiltin.RandomBytes, 0, args[0]);
        var count = args[0].GetInteger();
        if (count < 0 || count > 1024 * 1024)
            throw new InvalidOperationError(token, "crypto::random_bytes count must be between 0 and 1048576.");
        return Value.CreateBytes(RandomNumberGenerator.GetBytes((int)count));
    }

    /// <summary>A random (version 4) UUID in its lowercase 36-character form.</summary>
    private static Value Uuid(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CryptoBuiltin.Uuid, 0, args.Count);
        // Guid.NewGuid is not documented as cryptographically strong on every platform,
        // so set the RFC 9562 version and variant bits on secure random bytes instead.
        var b = RandomNumberGenerator.GetBytes(16);
        b[6] = (byte)((b[6] & 0x0F) | 0x40);
        b[8] = (byte)((b[8] & 0x3F) | 0x80);
        var hex = Convert.ToHexStringLower(b);
        return Value.CreateString($"{hex[..8]}-{hex[8..12]}-{hex[12..16]}-{hex[16..20]}-{hex[20..]}");
    }

    private static Value Hash(Token token, List<Value> args, string algorithm)
    {
        ParameterCountMismatchError.Check(token, algorithm, 1, args.Count);

        var input = GetData(token, algorithm, 0, args[0]);
        byte[] hash = algorithm switch
        {
            "MD5"    => MD5.HashData(input),
            "SHA1"   => SHA1.HashData(input),
            "SHA256" => SHA256.HashData(input),
            "SHA384" => SHA384.HashData(input),
            "SHA512" => SHA512.HashData(input),
            _ => throw new InvalidOperationException($"Unknown hash algorithm: {algorithm}")
        };

        return Hex(hash);
    }

    // Streams the file through the hash, so a large file isn't read into memory.
    private static Value FileHash(Token token, List<Value> args)
    {
        const string name = "file_hash";
        if (Kiwi.Settings.SafeMode) throw new SafeModeError(token, "crypto::file_hash");

        ParameterCountMismatchError.Check(token, name, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, name, 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, name, 1, args[1]);

        var path = args[0].GetString();
        var algorithm = args[1].GetString().ToUpperInvariant().Replace("-", "");
        HashAlgorithmName hashName = algorithm switch
        {
            "MD5"    => HashAlgorithmName.MD5,
            "SHA1"   => HashAlgorithmName.SHA1,
            "SHA256" => HashAlgorithmName.SHA256,
            "SHA384" => HashAlgorithmName.SHA384,
            "SHA512" => HashAlgorithmName.SHA512,
            _ => throw new InvalidOperationError(token, $"Unsupported hash algorithm '{args[1].GetString()}'. Use MD5, SHA1, SHA256, SHA384 or SHA512.")
        };

        try
        {
            using var hash = IncrementalHash.CreateHash(hashName);
            using var stream = File.OpenRead(path);
            var buffer = new byte[81920];
            int read;
            while ((read = stream.Read(buffer, 0, buffer.Length)) > 0)
                hash.AppendData(buffer, 0, read);
            return Hex(hash.GetHashAndReset());
        }
        catch (Exception ex) when (ex is IOException or UnauthorizedAccessException)
        {
            throw new FileReadError(token, path);
        }
    }

    private static Value HmacHash(Token token, List<Value> args, string algorithm)
    {
        ParameterCountMismatchError.Check(token, algorithm, 2, args.Count);

        var key   = GetData(token, algorithm, 0, args[0]);
        var input = GetData(token, algorithm, 1, args[1]);

        byte[] hash = algorithm switch
        {
            "HMACMD5"    => HMACMD5.HashData(key, input),
            "HMACSHA256" => HMACSHA256.HashData(key, input),
            "HMACSHA512" => HMACSHA512.HashData(key, input),
            _ => throw new InvalidOperationException($"Unknown HMAC algorithm: {algorithm}")
        };

        return Hex(hash);
    }

    // Strings are hashed as UTF-8, and bytes as they are.
    private static byte[] GetData(Token token, string name, int position, Value value)
    {
        if (value.IsBytes()) return value.GetBytes();
        if (value.IsString()) return Encoding.UTF8.GetBytes(value.GetString());
        throw new ParameterTypeMismatchError(token, name, position, value.Type, [Typing.ValueType.String, Typing.ValueType.Bytes]);
    }

    private static Value Hex(byte[] hash) => Value.CreateString(Convert.ToHexString(hash).ToLowerInvariant());
}
