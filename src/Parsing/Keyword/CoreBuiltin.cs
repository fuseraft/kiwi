
using System.Collections.Generic;

namespace kiwi.Parsing.Keyword;
public static class CoreBuiltin
{
    public const string Append       = "append";
    public const string Between      = "between";
    public const string Chars        = "chars";
    public const string Chomp        = "chomp";
    public const string IsA          = "is_a";
    public const string Join         = "join";
    public const string Split        = "split";
    public const string RSplit       = "rsplit";
    public const string Substring    = "substring";
    public const string LeftTrim     = "ltrim";
    public const string RightTrim    = "rtrim";
    public const string Trim         = "trim";
    public const string Size         = "size";
    public const string ToD          = "to_date";
    public const string ToF          = "to_float";
    public const string ToI          = "to_integer";
    public const string ToS          = "to_string";
    public const string ToBytes      = "to_bytes";
    public const string ToHex        = "to_hex";
    public const string Type         = "type";
    public const string Empty        = "empty";
    public const string Replace      = "replace";
    public const string RReplace     = "rreplace";
    public const string Reverse      = "reverse";
    public const string Contains     = "contains";
    public const string BeginsWith   = "begins_with";
    public const string EndsWith     = "ends_with";
    public const string IndexOf      = "index";
    public const string LastIndexOf  = "lastindex";
    public const string Uppercase    = "uppercase";
    public const string Lowercase    = "lowercase";
    public const string Keys         = "keys";
    public const string HasKey       = "has_key";
    public const string Members      = "members";
    public const string Push         = "push";
    public const string Pop          = "pop";
    public const string Enqueue      = "enqueue";
    public const string Dequeue      = "dequeue";
    public const string Shift        = "shift";
    public const string Unshift      = "unshift";
    public const string Clear        = "clear";
    public const string Remove       = "remove";
    public const string RemoveAt     = "remove_at";
    public const string Rotate       = "rotate";
    public const string Insert       = "insert";
    public const string Slice        = "slice";
    public const string Concat       = "concat";
    public const string Unique       = "unique";
    public const string Count        = "count";
    public const string Flatten      = "flatten";
    public const string Zip          = "zip";
    public const string Merge        = "merge";
    public const string Values       = "values";
    public const string Clone        = "clone";
    public const string Pretty       = "pretty";
    public const string Find         = "find";
    public const string Match        = "match";
    public const string Matches      = "matches";
    public const string MatchesAll   = "matches_all";
    public const string Scan         = "scan";
    public const string Set          = "set";
    public const string Swap         = "swap";
    public const string Get          = "get";
    public const string First        = "first";
    public const string Last         = "last";
    public const string Truthy       = "truthy";
    public const string Lines        = "lines";
    public const string Tokens       = "tokens";
    public const string Hour         = "hour";
    public const string Minute       = "minute";
    public const string Second       = "second";
    public const string Millisecond  = "millisecond";
    public const string Day          = "day";
    public const string Month        = "month";
    public const string Year         = "year";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Append,       TokenName.Builtin_Core_Append      },
            { Between,      TokenName.Builtin_Core_Between     },
            { Chars,        TokenName.Builtin_Core_Chars       },
            { Chomp,        TokenName.Builtin_Core_Chomp       },
            { IsA,          TokenName.Builtin_Core_IsA         },
            { Join,         TokenName.Builtin_Core_Join        },
            { Split,        TokenName.Builtin_Core_Split       },
            { RSplit,       TokenName.Builtin_Core_RSplit      },
            { Substring,    TokenName.Builtin_Core_Substring   },
            { LeftTrim,     TokenName.Builtin_Core_LeftTrim    },
            { RightTrim,    TokenName.Builtin_Core_RightTrim   },
            { Trim,         TokenName.Builtin_Core_Trim        },
            { Size,         TokenName.Builtin_Core_Size        },
            { ToD,          TokenName.Builtin_Core_ToDate      },
            { ToF,          TokenName.Builtin_Core_ToFloat     },
            { ToI,          TokenName.Builtin_Core_ToInteger   },
            { ToS,          TokenName.Builtin_Core_ToString    },
            { ToBytes,      TokenName.Builtin_Core_ToBytes     },
            { ToHex,        TokenName.Builtin_Core_ToHex       },
            { Type,         TokenName.Builtin_Core_Type        },
            { Empty,        TokenName.Builtin_Core_Empty       },
            { Replace,      TokenName.Builtin_Core_Replace     },
            { RReplace,     TokenName.Builtin_Core_RReplace    },
            { Reverse,      TokenName.Builtin_Core_Reverse     },
            { Contains,     TokenName.Builtin_Core_Contains    },
            { BeginsWith,   TokenName.Builtin_Core_BeginsWith  },
            { EndsWith,     TokenName.Builtin_Core_EndsWith    },
            { IndexOf,      TokenName.Builtin_Core_IndexOf     },
            { LastIndexOf,  TokenName.Builtin_Core_LastIndexOf },
            { Uppercase,    TokenName.Builtin_Core_Uppercase   },
            { Lowercase,    TokenName.Builtin_Core_Lowercase   },
            { Keys,         TokenName.Builtin_Core_Keys        },
            { HasKey,       TokenName.Builtin_Core_HasKey      },
            { Members,      TokenName.Builtin_Core_Members     },
            { Push,         TokenName.Builtin_Core_Push        },
            { Pop,          TokenName.Builtin_Core_Pop         },
            { Enqueue,      TokenName.Builtin_Core_Enqueue     },
            { Dequeue,      TokenName.Builtin_Core_Dequeue     },
            { Shift,        TokenName.Builtin_Core_Shift       },
            { Unshift,      TokenName.Builtin_Core_Unshift     },
            { Clear,        TokenName.Builtin_Core_Clear       },
            { Remove,       TokenName.Builtin_Core_Remove      },
            { RemoveAt,     TokenName.Builtin_Core_RemoveAt    },
            { Rotate,       TokenName.Builtin_Core_Rotate      },
            { Insert,       TokenName.Builtin_Core_Insert      },
            { Slice,        TokenName.Builtin_Core_Slice       },
            { Concat,       TokenName.Builtin_Core_Concat      },
            { Unique,       TokenName.Builtin_Core_Unique      },
            { Count,        TokenName.Builtin_Core_Count       },
            { Flatten,      TokenName.Builtin_Core_Flatten     },
            { Zip,          TokenName.Builtin_Core_Zip         },
            { Merge,        TokenName.Builtin_Core_Merge       },
            { Values,       TokenName.Builtin_Core_Values      },
            { Clone,        TokenName.Builtin_Core_Clone       },
            { Pretty,       TokenName.Builtin_Core_Pretty      },
            { Find,         TokenName.Builtin_Core_Find        },
            { Match,        TokenName.Builtin_Core_Match       },
            { Matches,      TokenName.Builtin_Core_Matches     },
            { MatchesAll,   TokenName.Builtin_Core_MatchesAll  },
            { Scan,         TokenName.Builtin_Core_Scan        },
            { Set,          TokenName.Builtin_Core_Set         },
            { Swap,         TokenName.Builtin_Core_Swap        },
            { Get,          TokenName.Builtin_Core_Get         },
            { First,        TokenName.Builtin_Core_First       },
            { Last,         TokenName.Builtin_Core_Last        },
            { Truthy,       TokenName.Builtin_Core_Truthy      },
            { Lines,        TokenName.Builtin_Core_Lines       },
            { Tokens,       TokenName.Builtin_Core_Tokens      },
            { Hour,         TokenName.Builtin_Core_Hour        },
            { Minute,       TokenName.Builtin_Core_Minute      },
            { Second,       TokenName.Builtin_Core_Second      },
            { Millisecond,  TokenName.Builtin_Core_Millisecond },
            { Day,          TokenName.Builtin_Core_Day         },
            { Month,        TokenName.Builtin_Core_Month       },
            { Year,         TokenName.Builtin_Core_Year        },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();
    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(string arg)
    {
        if (ListBuiltin.Map.ContainsKey(arg))
        {
            return true;
        }

        return Map.ContainsKey(arg);
    }

    public static bool IsBuiltin(TokenName arg)
    {
        if (ListBuiltin.IsBuiltin(arg))
        {
            return true;
        }

        return _names.Contains(arg);
    }

    public static bool IsBuiltinMethod(string arg)
    {
        if (ConsoleBuiltin.Map.ContainsKey(arg))    return true;
        if (EnvBuiltin.Map.ContainsKey(arg))        return true;
        if (TimeBuiltin.Map.ContainsKey(arg))       return true;
        if (FileIOBuiltin.Map.ContainsKey(arg))     return true;
        if (MathBuiltin.Map.ContainsKey(arg))       return true;
        if (SysBuiltin.Map.ContainsKey(arg))        return true;
        if (HttpBuiltin.Map.ContainsKey(arg))       return true;
        if (WebServerBuiltin.Map.ContainsKey(arg))  return true;
        if (LoggingBuiltin.Map.ContainsKey(arg))    return true;
        if (EncoderBuiltin.Map.ContainsKey(arg))    return true;
        if (SerializerBuiltin.Map.ContainsKey(arg)) return true;
        if (StdInBuiltin.Map.ContainsKey(arg))      return true;
        if (FFIBuiltin.Map.ContainsKey(arg))        return true;
        if (ReflectorBuiltin.Map.ContainsKey(arg))  return true;
        if (SignalBuiltin.Map.ContainsKey(arg))     return true;
        if (SocketBuiltin.Map.ContainsKey(arg))     return true;
        if (TaskBuiltin.Map.ContainsKey(arg))       return true;

        // not a builtin method
        return false;
    }
}
