namespace kiwi.Parsing.Keyword;

public static class MutexBuiltin
{
    public const string Acquire    = "__mutex_acquire__";
    public const string Create     = "__mutex_create__";
    public const string Destroy    = "__mutex_destroy__";
    public const string Release    = "__mutex_release__";
    public const string TryAcquire = "__mutex_tryacquire__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Acquire,    TokenName.Builtin_Mutex_Acquire    },
            { Create,     TokenName.Builtin_Mutex_Create     },
            { Destroy,    TokenName.Builtin_Mutex_Destroy    },
            { Release,    TokenName.Builtin_Mutex_Release    },
            { TryAcquire, TokenName.Builtin_Mutex_TryAcquire },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
