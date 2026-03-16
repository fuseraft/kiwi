namespace kiwi.Parsing.Keyword;

public static class SysBuiltin
{
    public const string Exec            = "__sys_exec__";
    public const string ExecOut         = "__sys_execout__";
    public const string Kill            = "__sys_kill__";
    public const string Open            = "__sys_open__";
    public const string Pid             = "__sys_pid__";
    public const string Pids            = "__sys_pids__";
    public const string Processes       = "__sys_processes__";
    public const string Running         = "__sys_running__";
    public const string Spawn           = "__sys_spawn__";
    public const string Wait            = "__sys_wait__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Exec,            TokenName.Builtin_Sys_Exec },
            { ExecOut,         TokenName.Builtin_Sys_ExecOut },
            { Kill,            TokenName.Builtin_Sys_Kill },
            { Open,            TokenName.Builtin_Sys_Open },
            { Pid,             TokenName.Builtin_Sys_Pid },
            { Pids,            TokenName.Builtin_Sys_Pids },
            { Processes,       TokenName.Builtin_Sys_Processes },
            { Running,         TokenName.Builtin_Sys_Running },
            { Spawn,           TokenName.Builtin_Sys_Spawn },
            { Wait,            TokenName.Builtin_Sys_Wait },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
