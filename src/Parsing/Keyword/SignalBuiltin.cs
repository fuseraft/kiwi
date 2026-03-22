namespace kiwi.Parsing.Keyword;

public static class SignalBuiltin
{
    public const string Check = "__signal_check__";
    public const string Raise = "__signal_raise__";
    public const string Reset = "__signal_reset__";
    public const string Trap  = "__signal_trap__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Check, TokenName.Builtin_Signal_Check },
            { Raise, TokenName.Builtin_Signal_Raise },
            { Reset, TokenName.Builtin_Signal_Reset },
            { Trap,  TokenName.Builtin_Signal_Trap  },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
