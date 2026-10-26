namespace kiwi.Parsing.Keyword;

public static class HttpServerBuiltin
{
    public const string Accept  = "__httpserver_accept__";
    public const string Create  = "__httpserver_create__";
    public const string Respond = "__httpserver_respond__";
    public const string Stop    = "__httpserver_stop__";
    public const string Stream  = "__httpserver_stream__";
    public const string Write   = "__httpserver_write__";
    public const string End     = "__httpserver_end__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Accept,  TokenName.Builtin_HttpServer_Accept  },
            { Create,  TokenName.Builtin_HttpServer_Create  },
            { Respond, TokenName.Builtin_HttpServer_Respond },
            { Stop,    TokenName.Builtin_HttpServer_Stop    },
            { Stream,  TokenName.Builtin_HttpServer_Stream  },
            { Write,   TokenName.Builtin_HttpServer_Write   },
            { End,     TokenName.Builtin_HttpServer_End     },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
