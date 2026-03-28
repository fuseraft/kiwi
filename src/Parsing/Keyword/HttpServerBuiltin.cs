namespace kiwi.Parsing.Keyword;

public static class HttpServerBuiltin
{
    public const string Accept  = "__httpserver_accept__";
    public const string Create  = "__httpserver_create__";
    public const string Respond = "__httpserver_respond__";
    public const string Stop    = "__httpserver_stop__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Accept,  TokenName.Builtin_HttpServer_Accept  },
            { Create,  TokenName.Builtin_HttpServer_Create  },
            { Respond, TokenName.Builtin_HttpServer_Respond },
            { Stop,    TokenName.Builtin_HttpServer_Stop    },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
