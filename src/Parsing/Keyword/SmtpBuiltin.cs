namespace kiwi.Parsing.Keyword;

public static class SmtpBuiltin
{
    public const string Send = "__smtp_send__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Send, TokenName.Builtin_Smtp_Send },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
