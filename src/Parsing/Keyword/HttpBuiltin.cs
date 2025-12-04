namespace kiwi.Parsing.Keyword;

public static class HttpBuiltin
{
    public const string Get    = "__webc_get__";
    public const string Post   = "__webc_post__";
    public const string Put    = "__webc_put__";
    public const string Delete = "__webc_delete__";
    public const string Patch  = "__webc_patch__";
    public const string Head   = "__webc_head__";
    public const string Options = "__webc_options__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Get,    TokenName.Builtin_WebClient_Get },
            { Post,   TokenName.Builtin_WebClient_Post },
            { Put,    TokenName.Builtin_WebClient_Put },
            { Delete, TokenName.Builtin_WebClient_Delete },
            { Patch,  TokenName.Builtin_WebClient_Patch },
            { Head,   TokenName.Builtin_WebClient_Head },
            { Options,TokenName.Builtin_WebClient_Options }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
