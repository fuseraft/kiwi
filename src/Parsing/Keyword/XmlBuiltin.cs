namespace kiwi.Parsing.Keyword;

public static class XmlBuiltin
{
    public const string Parse     = "__xml_parse__";
    public const string ParseFile = "__xml_parsefile__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Parse,     TokenName.Builtin_Xml_Parse     },
            { ParseFile, TokenName.Builtin_Xml_ParseFile },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
