namespace kiwi.Parsing.Keyword;

public static class JsonBuiltin
{
    public const string Parse     = "__json_parse__";
    public const string Stringify = "__json_stringify__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Parse,     TokenName.Builtin_Json_Parse },
            { Stringify, TokenName.Builtin_Json_Stringify },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
