namespace kiwi.Parsing.Keyword;

public static class CryptoBuiltin
{
    public const string FileHash   = "__crypto_file_hash__";
    public const string Md5        = "__crypto_md5__";
    public const string Sha1       = "__crypto_sha1__";
    public const string Sha256     = "__crypto_sha256__";
    public const string Sha384     = "__crypto_sha384__";
    public const string Sha512     = "__crypto_sha512__";
    public const string HmacMd5    = "__crypto_hmac_md5__";
    public const string HmacSha256 = "__crypto_hmac_sha256__";
    public const string HmacSha512 = "__crypto_hmac_sha512__";
    public const string RandomBytes = "__crypto_random_bytes__";
    public const string Uuid       = "__crypto_uuid__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { FileHash,   TokenName.Builtin_Crypto_FileHash },
            { Md5,        TokenName.Builtin_Crypto_Md5 },
            { Sha1,       TokenName.Builtin_Crypto_Sha1 },
            { Sha256,     TokenName.Builtin_Crypto_Sha256 },
            { Sha384,     TokenName.Builtin_Crypto_Sha384 },
            { Sha512,     TokenName.Builtin_Crypto_Sha512 },
            { HmacMd5,    TokenName.Builtin_Crypto_HmacMd5 },
            { HmacSha256, TokenName.Builtin_Crypto_HmacSha256 },
            { HmacSha512, TokenName.Builtin_Crypto_HmacSha512 },
            { RandomBytes, TokenName.Builtin_Crypto_RandomBytes },
            { Uuid,       TokenName.Builtin_Crypto_Uuid },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
