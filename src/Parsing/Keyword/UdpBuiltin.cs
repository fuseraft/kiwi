namespace kiwi.Parsing.Keyword;

public static class UdpBuiltin
{
    public const string Bind       = "__udp_bind__";
    public const string Close      = "__udp_close__";
    public const string Create     = "__udp_create__";
    public const string Recv       = "__udp_recv__";
    public const string Send       = "__udp_send__";
    public const string SetTimeout = "__udp_settimeout__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Bind,       TokenName.Builtin_Udp_Bind       },
            { Close,      TokenName.Builtin_Udp_Close      },
            { Create,     TokenName.Builtin_Udp_Create     },
            { Recv,       TokenName.Builtin_Udp_Recv       },
            { Send,       TokenName.Builtin_Udp_Send       },
            { SetTimeout, TokenName.Builtin_Udp_SetTimeout },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
