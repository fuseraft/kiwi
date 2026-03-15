namespace kiwi.Parsing.Keyword;

public static class CsvBuiltin
{
    public const string Parse          = "__csv_parse__";
    public const string ParseFile      = "__csv_parsefile__";
    public const string ParseMaps      = "__csv_parse_maps__";
    public const string ParseFileMaps  = "__csv_parsefile_maps__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Parse,         TokenName.Builtin_Csv_Parse },
            { ParseFile,     TokenName.Builtin_Csv_ParseFile },
            { ParseMaps,     TokenName.Builtin_Csv_ParseMaps },
            { ParseFileMaps, TokenName.Builtin_Csv_ParseFileMaps },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
