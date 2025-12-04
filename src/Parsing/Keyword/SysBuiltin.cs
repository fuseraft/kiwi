namespace kiwi.Parsing.Keyword;

public static class SysBuiltin
{
    public const string Exec            = "__sys_exec__";
    public const string ExecOut         = "__sys_execout__";
    public const string Open            = "__sys_open__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Exec,            TokenName.Builtin_Sys_Exec },
            { ExecOut,         TokenName.Builtin_Sys_ExecOut },
            { Open,            TokenName.Builtin_Sys_Open }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
