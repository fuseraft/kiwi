namespace kiwi.Parsing.Keyword;

public static class SqliteBuiltin
{
    public const string Open         = "__sqlite_open__";
    public const string Close        = "__sqlite_close__";
    public const string Exec         = "__sqlite_exec__";
    public const string ExecParams   = "__sqlite_exec_params__";
    public const string Query        = "__sqlite_query__";
    public const string QueryParams  = "__sqlite_query_params__";
    public const string LastInsertId = "__sqlite_last_insert_id__";
    public const string Changes      = "__sqlite_changes__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Open,         TokenName.Builtin_Sqlite_Open },
            { Close,        TokenName.Builtin_Sqlite_Close },
            { Exec,         TokenName.Builtin_Sqlite_Exec },
            { ExecParams,   TokenName.Builtin_Sqlite_ExecParams },
            { Query,        TokenName.Builtin_Sqlite_Query },
            { QueryParams,  TokenName.Builtin_Sqlite_QueryParams },
            { LastInsertId, TokenName.Builtin_Sqlite_LastInsertId },
            { Changes,      TokenName.Builtin_Sqlite_Changes },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
