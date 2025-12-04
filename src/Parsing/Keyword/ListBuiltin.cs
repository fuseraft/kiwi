namespace kiwi.Parsing.Keyword;

public static class ListBuiltin
{
    public const string All    = "all";
    public const string Each   = "each";
    public const string Map_   = "map";
    public const string None   = "none";
    public const string Reduce = "reduce";
    public const string Filter = "filter";
    public const string Sort   = "sort";
    public const string Sum    = "sum";
    public const string Min    = "min";
    public const string Max    = "max";
    public const string ToH    = "to_hashmap";
    public const string Skip   = "skip";
    public const string Take   = "take";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { All,    TokenName.Builtin_List_All },
            { Each,   TokenName.Builtin_List_Each },
            { Map_,   TokenName.Builtin_List_Map },
            { None,   TokenName.Builtin_List_None },
            { Reduce, TokenName.Builtin_List_Reduce },
            { Filter, TokenName.Builtin_List_Filter },
            { Sort,   TokenName.Builtin_List_Sort },
            { Sum,    TokenName.Builtin_List_Sum },
            { Min,    TokenName.Builtin_List_Min },
            { Max,    TokenName.Builtin_List_Max },
            { ToH,    TokenName.Builtin_List_ToH },
            { Skip,   TokenName.Builtin_List_Skip },
            { Take,   TokenName.Builtin_List_Take },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
