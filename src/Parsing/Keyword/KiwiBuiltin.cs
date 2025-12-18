namespace kiwi.Parsing.Keyword;

public static class KiwiBuiltin
{
    public const string TypeOf = "typeof";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { TypeOf, TokenName.Builtin_Kiwi_TypeOf },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
