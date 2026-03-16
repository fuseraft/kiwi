namespace kiwi.Parsing.Keyword;

public static class IpcBuiltin
{
    public const string Create  = "__ipc_create__";
    public const string Connect = "__ipc_connect__";
    public const string Send    = "__ipc_send__";
    public const string Recv    = "__ipc_recv__";
    public const string Close   = "__ipc_close__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Create,  TokenName.Builtin_Ipc_Create  },
            { Connect, TokenName.Builtin_Ipc_Connect },
            { Send,    TokenName.Builtin_Ipc_Send    },
            { Recv,    TokenName.Builtin_Ipc_Recv    },
            { Close,   TokenName.Builtin_Ipc_Close   },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
