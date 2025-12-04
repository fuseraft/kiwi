namespace kiwi.Parsing.Keyword;

public static class LoggingBuiltin
{
    public const string FilePath        = "__log_filepath__";
    public const string Mode            = "__log_mode__";
    public const string EntryFormat     = "__log_format__";
    public const string TimestampFormat = "__log_tsformat__";
    public const string Level           = "__log_level__";
    public const string Debug           = "__log_debug__";
    public const string Warn            = "__log_warn__";
    public const string Info            = "__log_info__";
    public const string Error           = "__log_error__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { FilePath,        TokenName.Builtin_Logging_FilePath },
            { Mode,            TokenName.Builtin_Logging_Mode },
            { EntryFormat,     TokenName.Builtin_Logging_EntryFormat },
            { TimestampFormat, TokenName.Builtin_Logging_TimestampFormat },
            { Level,           TokenName.Builtin_Logging_Level },
            { Debug,           TokenName.Builtin_Logging_Debug },
            { Warn,            TokenName.Builtin_Logging_Warn },
            { Info,            TokenName.Builtin_Logging_Info },
            { Error,           TokenName.Builtin_Logging_Error }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
