namespace kiwi.Parsing.Keyword;

public static class FFIBuiltin
{
    public const string Attach  = "__ffi_attach__";
    public const string Invoke  = "__ffi_invoke__";
    public const string Load    = "__ffi_load__";
    public const string Unload  = "__ffi_unload__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Attach, TokenName.Builtin_FFI_Attach },
            { Invoke, TokenName.Builtin_FFI_Invoke },
            { Load,   TokenName.Builtin_FFI_Load },
            { Unload, TokenName.Builtin_FFI_Unload }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
