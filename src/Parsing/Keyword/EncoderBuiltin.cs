namespace kiwi.Parsing.Keyword;

public static class EncoderBuiltin
{
    public const string Base64Encode = "__base64encode__";
    public const string Base64Decode = "__base64decode__";
    public const string UrlEncode    = "__urlencode__";
    public const string UrlDecode    = "__urldecode__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Base64Encode, TokenName.Builtin_Encoder_Base64Encode },
            { Base64Decode, TokenName.Builtin_Encoder_Base64Decode },
            { UrlEncode,    TokenName.Builtin_Encoder_UrlEncode },
            { UrlDecode,    TokenName.Builtin_Encoder_UrlDecode }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
