namespace kiwi.Parsing.Keyword;

public static class ConsoleBuiltin
{
    public const string Input = "input";
    public const string Background = "__console_background__";
    public const string Foreground = "__console_foreground__";
    public const string ReadKey = "__console_readkey__";
    public const string Reset = "__console_reset__";
    public const string Clear = "__console_clear__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Input,      TokenName.Builtin_Console_Input },
            { Background, TokenName.Builtin_Console_Background },
            { Foreground, TokenName.Builtin_Console_Foreground },
            { Clear,      TokenName.Builtin_Console_Clear },
            { ReadKey,    TokenName.Builtin_Console_ReadKey },
            { Reset,      TokenName.Builtin_Console_Reset }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
