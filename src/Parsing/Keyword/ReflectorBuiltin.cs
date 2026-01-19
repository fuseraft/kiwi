namespace kiwi.Parsing.Keyword;

public static class ReflectorBuiltin
{
    public const string RFFlags  = "__rfflags__";
    public const string RList    = "__rlist__";
    public const string RObject  = "__robject__";
    public const string RStack   = "__rstack__";
    public const string RRetVal  = "__rretval__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { RFFlags,  TokenName.Builtin_Reflector_RFFlags },
            { RList,    TokenName.Builtin_Reflector_RList },
            { RObject,  TokenName.Builtin_Reflector_RObject },
            { RStack,   TokenName.Builtin_Reflector_RStack },
            { RRetVal,  TokenName.Builtin_Reflector_RRetVal }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
