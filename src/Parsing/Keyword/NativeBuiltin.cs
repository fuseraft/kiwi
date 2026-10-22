namespace kiwi.Parsing.Keyword;

public static class NativeBuiltin
{
    public const string Load = "__native_load__";
    public const string Rid  = "__native_rid__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Load, TokenName.Builtin_Native_Load },
            { Rid,  TokenName.Builtin_Native_Rid  },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
