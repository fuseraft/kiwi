namespace kiwi.Parsing.Keyword;

public static class ReflectorBuiltin
{
    public const string CallStack     = "__reflector_callstack__";
    public const string FrameFlags    = "__reflector_fflags__";
    public const string GetFunc       = "__reflector_getfunc__";
    public const string ObjectContext = "__reflector_objectcontext__";
    public const string RetVal        = "__reflector_retval__";
    public const string State         = "__reflector_state__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { CallStack,     TokenName.Builtin_Reflector_CallStack },
            { FrameFlags,    TokenName.Builtin_Reflector_FrameFlags },
            { GetFunc,       TokenName.Builtin_Reflector_GetFunc },
            { ObjectContext, TokenName.Builtin_Reflector_ObjectContext },
            { RetVal,        TokenName.Builtin_Reflector_RetVal },
            { State,         TokenName.Builtin_Reflector_State },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
