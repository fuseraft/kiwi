namespace kiwi.Parsing.Keyword;

public static class HttpBuiltin
{
    public const string HttpDelete = "__http_delete__";
    public const string HttpDownload = "__http_download__";
    public const string HttpGet = "__http_get__";
    public const string HttpHead = "__http_head__";
    public const string HttpOptions = "__http_options__";
    public const string HttpPatch = "__http_patch__";
    public const string HttpPost = "__http_post__";
    public const string HttpPut = "__http_put__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { HttpDelete,   TokenName.Builtin_Http_Delete   },
            { HttpDownload, TokenName.Builtin_Http_Download },
            { HttpGet,      TokenName.Builtin_Http_Get      },
            { HttpHead,     TokenName.Builtin_Http_Head     },
            { HttpOptions,  TokenName.Builtin_Http_Options  },
            { HttpPatch,    TokenName.Builtin_Http_Patch    },
            { HttpPost,     TokenName.Builtin_Http_Post     },
            { HttpPut,      TokenName.Builtin_Http_Put      },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
