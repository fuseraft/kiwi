namespace kiwi.Parsing.Keyword;

public static class SocketBuiltin
{
    public const string Create      = "__socket_create__";
    public const string Bind        = "__socket_bind__";
    public const string Listen      = "__socket_listen__";
    public const string Accept      = "__socket_accept__";
    public const string Connect     = "__socket_connect__";
    public const string Send        = "__socket_send__";
    public const string SendRaw     = "__socket_sendraw__";
    public const string Receive     = "__socket_receive__";
    public const string Close       = "__socket_close__";
    public const string Shutdown    = "__socket_shutdown__";
    public const string TcpConnect  = "__socket_tcpconnect__";
    public const string TcpServer   = "__socket_tcpserver__";
    public const string ResolveHost = "__net_reshost__";
    public const string IsIPAddr    = "__net_isipaddr__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Create,      TokenName.Builtin_Socket_Create },
            { Bind,        TokenName.Builtin_Socket_Bind },
            { Listen,      TokenName.Builtin_Socket_Listen },
            { Accept,      TokenName.Builtin_Socket_Accept },
            { Connect,     TokenName.Builtin_Socket_Connect },
            { Send,        TokenName.Builtin_Socket_Send },
            { SendRaw,     TokenName.Builtin_Socket_SendRaw },
            { Receive,     TokenName.Builtin_Socket_Receive },
            { Close,       TokenName.Builtin_Socket_Close },
            { Shutdown,    TokenName.Builtin_Socket_Shutdown },
            { TcpConnect,  TokenName.Builtin_Socket_TcpConnect },
            { TcpServer,   TokenName.Builtin_Socket_TcpServer },
            { ResolveHost, TokenName.Builtin_Net_ResolveHost },
            { IsIPAddr,    TokenName.Builtin_Net_IsIPAddr }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
