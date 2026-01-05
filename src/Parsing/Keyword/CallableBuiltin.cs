namespace kiwi.Parsing.Keyword;

public static class CallableBuiltin
{
    public const string Call        = "call";
    public const string Parameters  = "parameters";
    public const string Returns     = "returns";
    public const string ToLambda    = "to_lambda";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Call,         TokenName.Builtin_Callable_Call       },
            { Parameters,   TokenName.Builtin_Callable_Parameters },
            { Returns,      TokenName.Builtin_Callable_Returns    },
            { ToLambda,     TokenName.Builtin_Callable_ToLambda   },
        };

    public static string MapName(TokenName name)
    {
        foreach (var kvp in _map)
        {
            if (kvp.Value == name)
            {
                return kvp.Key;
            }
        }

        return string.Empty;
    }

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
