namespace kiwi.Parsing.Keyword;

public static class EnvBuiltin
{
    public const string GetArgv                   = "__env_argv__";
    public const string GetXarg                   = "__env_xarg__";
    public const string OS                        = "__env_os__";
    public const string User                      = "__env_user__";
    public const string UserDomain                = "__env_userdomain__";
    public const string GetAll                    = "__env_getall__";
    public const string GetEnvironmentVariable    = "__env_getenv__";
    public const string SetEnvironmentVariable    = "__env_setenv__";
    public const string Kiwi                    = "__env_bin__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { GetArgv,                  TokenName.Builtin_Env_GetArgv },
            { GetXarg,                  TokenName.Builtin_Env_GetXarg },
            { GetAll,                   TokenName.Builtin_Env_GetAll },
            { OS,                       TokenName.Builtin_Env_OS },
            { User,                     TokenName.Builtin_Env_User },
            { UserDomain,               TokenName.Builtin_Env_UserDomain },
            { GetEnvironmentVariable,   TokenName.Builtin_Env_GetEnvironmentVariable },
            { SetEnvironmentVariable,   TokenName.Builtin_Env_SetEnvironmentVariable },
            { Kiwi,                   TokenName.Builtin_Env_Kiwi }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
