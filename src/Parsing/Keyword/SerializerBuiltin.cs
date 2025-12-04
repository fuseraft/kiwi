namespace kiwi.Parsing.Keyword;

public static class SerializerBuiltin
{
    public const string Deserialize = "deserialize";
    public const string Serialize   = "serialize";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Deserialize, TokenName.Builtin_Serializer_Deserialize },
            { Serialize,   TokenName.Builtin_Serializer_Serialize }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
