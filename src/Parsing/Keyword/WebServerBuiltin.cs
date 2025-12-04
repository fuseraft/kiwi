namespace kiwi.Parsing.Keyword;

public static class WebServerBuiltin
{
    public const string Get    = "__webs_get__";
    public const string Post   = "__webs_post__";
    public const string Listen = "__webs_listen__";
    public const string Host   = "__webs_host__";
    public const string Port   = "__webs_port__";
    public const string Public = "__webs_public__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Get,    TokenName.Builtin_WebServer_Get },
            { Post,   TokenName.Builtin_WebServer_Post },
            { Listen, TokenName.Builtin_WebServer_Listen },
            { Host,   TokenName.Builtin_WebServer_Host },
            { Port,   TokenName.Builtin_WebServer_Port },
            { Public, TokenName.Builtin_WebServer_Public }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
