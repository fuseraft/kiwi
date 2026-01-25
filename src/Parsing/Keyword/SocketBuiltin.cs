namespace kiwi.Parsing.Keyword;

public static class SocketBuiltin
{
    public const string Accept      = "__socket_accept__";
    public const string Send        = "__socket_send__";
    public const string Receive     = "__socket_recv__";
    public const string Close       = "__socket_close__";
    public const string TcpConnect  = "__socket_tcpconnect__";
    public const string TcpServer   = "__socket_tcpserver__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Accept,      TokenName.Builtin_Socket_Accept },
            { Send,        TokenName.Builtin_Socket_Send },
            { Receive,     TokenName.Builtin_Socket_Receive },
            { Close,       TokenName.Builtin_Socket_Close },
            { TcpConnect,  TokenName.Builtin_Socket_TcpConnect },
            { TcpServer,   TokenName.Builtin_Socket_TcpServer },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
