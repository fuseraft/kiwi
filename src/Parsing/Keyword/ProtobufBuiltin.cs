namespace kiwi.Parsing.Keyword;

public static class ProtobufBuiltin
{
    public const string Encode = "__protobuf_encode__";
    public const string Decode = "__protobuf_decode__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Encode, TokenName.Builtin_Protobuf_Encode },
            { Decode, TokenName.Builtin_Protobuf_Decode },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
