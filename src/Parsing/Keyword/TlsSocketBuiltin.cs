namespace kiwi.Parsing.Keyword;

public static class TlsSocketBuiltin
{
    public const string Accept      = "__tlssocket_accept__";
    public const string Send        = "__tlssocket_send__";
    public const string Receive     = "__tlssocket_recv__";
    public const string Close       = "__tlssocket_close__";
    public const string TcpConnect  = "__tlssocket_tcpconnect__";
    public const string TcpServer   = "__tlssocket_tcpserver__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Accept,      TokenName.Builtin_Tls_Accept },
            { Send,        TokenName.Builtin_Tls_Send },
            { Receive,     TokenName.Builtin_Tls_Receive },
            { Close,       TokenName.Builtin_Tls_Close },
            { TcpConnect,  TokenName.Builtin_Tls_TcpConnect },
            { TcpServer,   TokenName.Builtin_Tls_TcpServer },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
