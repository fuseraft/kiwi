namespace kiwi.Parsing.Keyword;

public static class StdInBuiltin
{
    public const string Read = "__stdin_read__";
    public const string ReadBytes = "__stdin_readbytes__";
    public const string ReadLine = "__stdin_readline__";
    public const string Lines = "__stdin_lines__";
    public const string Empty = "__stdin_empty__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Read,       TokenName.Builtin_Stdin_Read },
            { ReadBytes,  TokenName.Builtin_Stdin_ReadBytes },
            { ReadLine,   TokenName.Builtin_Stdin_ReadLine },
            { Lines,      TokenName.Builtin_Stdin_Lines },
            { Empty,      TokenName.Builtin_Stdin_Empty }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
