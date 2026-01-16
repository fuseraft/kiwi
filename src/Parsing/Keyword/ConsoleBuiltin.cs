namespace kiwi.Parsing.Keyword;

public static class ConsoleBuiltin
{
    public const string Clear = "__console_clear__";
    public const string CursorVisible = "__console_cursorvisible__";
    public const string Input = "input";
    public const string Background = "__console_background__";
    public const string Foreground = "__console_foreground__";
    public const string ReadKey = "__console_readkey__";
    public const string Reset = "__console_reset__";
    public const string Title = "__console_title__";
    public const string WindowSize = "__console_windowsize__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Clear,            TokenName.Builtin_Console_Clear },
            { CursorVisible,    TokenName.Builtin_Console_CursorVisible },
            { Input,            TokenName.Builtin_Console_Input },
            { Background,       TokenName.Builtin_Console_Background },
            { Foreground,       TokenName.Builtin_Console_Foreground },
            { ReadKey,          TokenName.Builtin_Console_ReadKey },
            { Reset,            TokenName.Builtin_Console_Reset },
            { Title,            TokenName.Builtin_Console_Title },
            { WindowSize,       TokenName.Builtin_Console_WindowSize }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
