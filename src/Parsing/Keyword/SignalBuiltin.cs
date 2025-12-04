namespace kiwi.Parsing.Keyword;

public static class SignalBuiltin
{
    public const string Raise = "__sig_raise__";
    public const string Send  = "__sig_send__";
    public const string Trap  = "__sig_trap__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Raise, TokenName.Builtin_Signal_Raise },
            { Send,  TokenName.Builtin_Signal_Send },
            { Trap,  TokenName.Builtin_Signal_Trap }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
