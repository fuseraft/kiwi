namespace kiwi.Parsing.Keyword;

public static class TaskBuiltin
{
    public const string TaskBusy = "__task_busy__";
    public const string TaskList = "__task_list__";
    public const string TaskResult = "__task_result__";
    public const string TaskSleep = "__task_sleep__";
    public const string TaskStatus = "__task_status__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { TaskBusy,   TokenName.Builtin_Task_Busy },
            { TaskList,   TokenName.Builtin_Task_List },
            { TaskResult, TokenName.Builtin_Task_Result },
            { TaskSleep,  TokenName.Builtin_Task_Sleep },
            { TaskStatus, TokenName.Builtin_Task_Status }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
