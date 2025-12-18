namespace kiwi.Parsing.Keyword;

public static class CoreBuiltin
{
    public const string Append       = "append";
    public const string BeginsWith   = "begins_with";
    public const string Between      = "between";
    public const string Chars        = "chars";
    public const string Chomp        = "chomp";
    public const string Clear        = "clear";
    public const string Clone        = "clone";
    public const string Concat       = "concat";
    public const string Contains     = "contains";
    public const string Count        = "count";
    public const string Day          = "day";
    public const string Dequeue      = "dequeue";
    public const string Empty        = "empty";
    public const string EndsWith     = "ends_with";
    public const string Enqueue      = "enqueue";
    public const string Find         = "find";
    public const string First        = "first";
    public const string Flatten      = "flatten";
    public const string Get          = "get";
    public const string HasKey       = "has_key";
    public const string Hour         = "hour";
    public const string IndexOf      = "index";
    public const string Insert       = "insert";
    public const string IsA          = "is_a";
    public const string Join         = "join";
    public const string Keys         = "keys";
    public const string Last         = "last";
    public const string LastIndexOf  = "lastindex";
    public const string LeftTrim     = "ltrim";
    public const string Lines        = "lines";
    public const string Lowercase    = "lowercase";
    public const string Match        = "match";
    public const string Matches      = "matches";
    public const string MatchesAll   = "matches_all";
    public const string Members      = "members";
    public const string Merge        = "merge";
    public const string Millisecond  = "millisecond";
    public const string Minute       = "minute";
    public const string Month        = "month";
    public const string Pop          = "pop";
    public const string Pretty       = "pretty";
    public const string Push         = "push";
    public const string Remove       = "remove";
    public const string RemoveAt     = "remove_at";
    public const string Replace      = "replace";
    public const string Reverse      = "reverse";
    public const string RightTrim    = "rtrim";
    public const string Rotate       = "rotate";
    public const string RReplace     = "rreplace";
    public const string RSplit       = "rsplit";
    public const string Scan         = "scan";
    public const string Second       = "second";
    public const string Set          = "set";
    public const string Shift        = "shift";
    public const string Size         = "size";
    public const string Slice        = "slice";
    public const string Split        = "split";
    public const string Substring    = "substring";
    public const string Swap         = "swap";
    public const string ToBytes      = "to_bytes";
    public const string ToD          = "to_date";
    public const string ToF          = "to_float";
    public const string ToHex        = "to_hex";
    public const string ToI          = "to_integer";
    public const string Tokens       = "tokens";
    public const string ToS          = "to_string";
    public const string Trim         = "trim";
    public const string Truthy       = "truthy";
    public const string Type         = "type";
    public const string Unique       = "unique";
    public const string Unshift      = "unshift";
    public const string Uppercase    = "uppercase";
    public const string Values       = "values";
    public const string Year         = "year";
    public const string Zip          = "zip";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Append,       TokenName.Builtin_Core_Append      },
            { BeginsWith,   TokenName.Builtin_Core_BeginsWith  },
            { Between,      TokenName.Builtin_Core_Between     },
            { Chars,        TokenName.Builtin_Core_Chars       },
            { Chomp,        TokenName.Builtin_Core_Chomp       },
            { Clear,        TokenName.Builtin_Core_Clear       },
            { Clone,        TokenName.Builtin_Core_Clone       },
            { Concat,       TokenName.Builtin_Core_Concat      },
            { Contains,     TokenName.Builtin_Core_Contains    },
            { Count,        TokenName.Builtin_Core_Count       },
            { Day,          TokenName.Builtin_Core_Day         },
            { Dequeue,      TokenName.Builtin_Core_Dequeue     },
            { Empty,        TokenName.Builtin_Core_Empty       },
            { EndsWith,     TokenName.Builtin_Core_EndsWith    },
            { Enqueue,      TokenName.Builtin_Core_Enqueue     },
            { Find,         TokenName.Builtin_Core_Find        },
            { First,        TokenName.Builtin_Core_First       },
            { Flatten,      TokenName.Builtin_Core_Flatten     },
            { Get,          TokenName.Builtin_Core_Get         },
            { HasKey,       TokenName.Builtin_Core_HasKey      },
            { Hour,         TokenName.Builtin_Core_Hour        },
            { IndexOf,      TokenName.Builtin_Core_IndexOf     },
            { Insert,       TokenName.Builtin_Core_Insert      },
            { IsA,          TokenName.Builtin_Core_IsA         },
            { Join,         TokenName.Builtin_Core_Join        },
            { Keys,         TokenName.Builtin_Core_Keys        },
            { Last,         TokenName.Builtin_Core_Last        },
            { LastIndexOf,  TokenName.Builtin_Core_LastIndexOf },
            { LeftTrim,     TokenName.Builtin_Core_LeftTrim    },
            { Lines,        TokenName.Builtin_Core_Lines       },
            { Lowercase,    TokenName.Builtin_Core_Lowercase   },
            { Match,        TokenName.Builtin_Core_Match       },
            { Matches,      TokenName.Builtin_Core_Matches     },
            { MatchesAll,   TokenName.Builtin_Core_MatchesAll  },
            { Members,      TokenName.Builtin_Core_Members     },
            { Merge,        TokenName.Builtin_Core_Merge       },
            { Millisecond,  TokenName.Builtin_Core_Millisecond },
            { Minute,       TokenName.Builtin_Core_Minute      },
            { Month,        TokenName.Builtin_Core_Month       },
            { Pop,          TokenName.Builtin_Core_Pop         },
            { Pretty,       TokenName.Builtin_Core_Pretty      },
            { Push,         TokenName.Builtin_Core_Push        },
            { Remove,       TokenName.Builtin_Core_Remove      },
            { RemoveAt,     TokenName.Builtin_Core_RemoveAt    },
            { Replace,      TokenName.Builtin_Core_Replace     },
            { Reverse,      TokenName.Builtin_Core_Reverse     },
            { RightTrim,    TokenName.Builtin_Core_RightTrim   },
            { Rotate,       TokenName.Builtin_Core_Rotate      },
            { RReplace,     TokenName.Builtin_Core_RReplace    },
            { RSplit,       TokenName.Builtin_Core_RSplit      },
            { Scan,         TokenName.Builtin_Core_Scan        },
            { Second,       TokenName.Builtin_Core_Second      },
            { Set,          TokenName.Builtin_Core_Set         },
            { Shift,        TokenName.Builtin_Core_Shift       },
            { Size,         TokenName.Builtin_Core_Size        },
            { Slice,        TokenName.Builtin_Core_Slice       },
            { Split,        TokenName.Builtin_Core_Split       },
            { Substring,    TokenName.Builtin_Core_Substring   },
            { Swap,         TokenName.Builtin_Core_Swap        },
            { ToBytes,      TokenName.Builtin_Core_ToBytes     },
            { ToD,          TokenName.Builtin_Core_ToDate      },
            { ToF,          TokenName.Builtin_Core_ToFloat     },
            { ToHex,        TokenName.Builtin_Core_ToHex       },
            { ToI,          TokenName.Builtin_Core_ToInteger   },
            { Tokens,       TokenName.Builtin_Core_Tokens      },
            { ToS,          TokenName.Builtin_Core_ToString    },
            { Trim,         TokenName.Builtin_Core_Trim        },
            { Truthy,       TokenName.Builtin_Core_Truthy      },
            { Type,         TokenName.Builtin_Core_Type        },
            { Unique,       TokenName.Builtin_Core_Unique      },
            { Unshift,      TokenName.Builtin_Core_Unshift     },
            { Uppercase,    TokenName.Builtin_Core_Uppercase   },
            { Values,       TokenName.Builtin_Core_Values      },
            { Year,         TokenName.Builtin_Core_Year        },
            { Zip,          TokenName.Builtin_Core_Zip         },
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
        if (EncoderBuiltin.Map.ContainsKey(arg))    return true;
        if (EnvBuiltin.Map.ContainsKey(arg))        return true;
        if (FFIBuiltin.Map.ContainsKey(arg))        return true;
        if (FileIOBuiltin.Map.ContainsKey(arg))     return true;
        if (HttpBuiltin.Map.ContainsKey(arg))       return true;
        if (KiwiBuiltin.Map.ContainsKey(arg))       return true;
        if (LoggingBuiltin.Map.ContainsKey(arg))    return true;
        if (MathBuiltin.Map.ContainsKey(arg))       return true;
        if (ReflectorBuiltin.Map.ContainsKey(arg))  return true;
        if (SerializerBuiltin.Map.ContainsKey(arg)) return true;
        if (SignalBuiltin.Map.ContainsKey(arg))     return true;
        if (SocketBuiltin.Map.ContainsKey(arg))     return true;
        if (StdInBuiltin.Map.ContainsKey(arg))      return true;
        if (SysBuiltin.Map.ContainsKey(arg))        return true;
        if (TaskBuiltin.Map.ContainsKey(arg))       return true;
        if (TimeBuiltin.Map.ContainsKey(arg))       return true;
        if (WebServerBuiltin.Map.ContainsKey(arg))  return true;

        // not a builtin method
        return false;
    }
}
