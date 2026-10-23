namespace kiwi.Parsing.Keyword;

public static class FfiBuiltin
{
    public const string Open       = "__ffi_open__";
    public const string Close      = "__ffi_close__";
    public const string Symbol     = "__ffi_symbol__";
    public const string Signature  = "__ffi_signature__";
    public const string Call       = "__ffi_call__";
    public const string Alloc      = "__ffi_alloc__";
    public const string Free       = "__ffi_free__";
    public const string Read       = "__ffi_read__";
    public const string Write      = "__ffi_write__";
    public const string String     = "__ffi_string__";
    public const string Bytes      = "__ffi_bytes__";
    public const string WriteBytes = "__ffi_write_bytes__";
    public const string CString    = "__ffi_cstring__";
    public const string SizeOf     = "__ffi_sizeof__";
    public const string Offset     = "__ffi_offset__";
    public const string Address    = "__ffi_address__";
    public const string Pointer    = "__ffi_pointer__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Open,       TokenName.Builtin_Ffi_Open       },
            { Close,      TokenName.Builtin_Ffi_Close      },
            { Symbol,     TokenName.Builtin_Ffi_Symbol     },
            { Signature,  TokenName.Builtin_Ffi_Signature  },
            { Call,       TokenName.Builtin_Ffi_Call       },
            { Alloc,      TokenName.Builtin_Ffi_Alloc      },
            { Free,       TokenName.Builtin_Ffi_Free       },
            { Read,       TokenName.Builtin_Ffi_Read       },
            { Write,      TokenName.Builtin_Ffi_Write      },
            { String,     TokenName.Builtin_Ffi_String     },
            { Bytes,      TokenName.Builtin_Ffi_Bytes      },
            { WriteBytes, TokenName.Builtin_Ffi_WriteBytes },
            { CString,    TokenName.Builtin_Ffi_CString    },
            { SizeOf,     TokenName.Builtin_Ffi_SizeOf     },
            { Offset,     TokenName.Builtin_Ffi_Offset     },
            { Address,    TokenName.Builtin_Ffi_Address    },
            { Pointer,    TokenName.Builtin_Ffi_Pointer    },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
