namespace kiwi.Parsing.Keyword;

public static class MathBuiltin
{
    public const string Sin    = "__sin__";
    public const string Tan    = "__tan__";
    public const string Asin   = "__asin__";
    public const string Acos   = "__acos__";
    public const string Atan   = "__atan__";
    public const string Atan2  = "__atan2__";
    public const string Sinh   = "__sinh__";
    public const string Cosh   = "__cosh__";
    public const string Tanh   = "__tanh__";
    public const string Cos    = "__cos__";
    public const string Log    = "__log__";
    public const string Log2   = "__log2__";
    public const string Log10  = "__log10__";
    public const string Log1P  = "__log1p__";
    public const string Fmod   = "__fmod__";
    public const string Hypot  = "__hypot__";
    public const string IsFinite = "__isfinite__";
    public const string IsInf    = "__isinf__";
    public const string IsNaN    = "__isnan__";
    public const string IsNormal = "__isnormal__";
    public const string Sqrt     = "__sqrt__";
    public const string Cbrt     = "__cbrt__";
    public const string Abs      = "__abs__";
    public const string Floor    = "__floor__";
    public const string Ceil     = "__ceil__";
    public const string Round    = "__round__";
    public const string Trunc    = "__trunc__";
    public const string Remainder = "__remainder__";
    public const string Exp      = "__exp__";
    public const string ExpM1    = "__expm1__";
    public const string Erf      = "__erf__";
    public const string ErfC     = "__erfc__";
    public const string LGamma   = "__lgamma__";
    public const string TGamma   = "__tgamma__";
    public const string FMax     = "__fmax__";
    public const string FMin     = "__fmin__";
    public const string FDim     = "__fdim__";
    public const string CopySign = "__copysign__";
    public const string NextAfter = "__nextafter__";
    public const string Pow      = "__pow__";
    public const string Epsilon  = "__eps__";
    public const string Random   = "__random__";
    public const string RotateLeft  = "__rotl__";
    public const string RotateRight = "__rotr__";
    public const string Divisors    = "__divisors__";
    public const string ListPrimes  = "__listprimes__";
    public const string NthPrime    = "__nthprime__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Sin,         TokenName.Builtin_Math_Sin },
            { Tan,         TokenName.Builtin_Math_Tan },
            { Asin,        TokenName.Builtin_Math_Asin },
            { Acos,        TokenName.Builtin_Math_Acos },
            { Atan,        TokenName.Builtin_Math_Atan },
            { Atan2,       TokenName.Builtin_Math_Atan2 },
            { Sinh,        TokenName.Builtin_Math_Sinh },
            { Cosh,        TokenName.Builtin_Math_Cosh },
            { Tanh,        TokenName.Builtin_Math_Tanh },
            { Cos,         TokenName.Builtin_Math_Cos },
            { Log,         TokenName.Builtin_Math_Log },
            { Log2,        TokenName.Builtin_Math_Log2 },
            { Log10,       TokenName.Builtin_Math_Log10 },
            { Log1P,       TokenName.Builtin_Math_Log1P },
            { Fmod,        TokenName.Builtin_Math_Fmod },
            { Hypot,       TokenName.Builtin_Math_Hypot },
            { IsFinite,    TokenName.Builtin_Math_IsFinite },
            { IsInf,       TokenName.Builtin_Math_IsInf },
            { IsNaN,       TokenName.Builtin_Math_IsNaN },
            { IsNormal,    TokenName.Builtin_Math_IsNormal },
            { Sqrt,        TokenName.Builtin_Math_Sqrt },
            { Cbrt,        TokenName.Builtin_Math_Cbrt },
            { Abs,         TokenName.Builtin_Math_Abs },
            { Floor,       TokenName.Builtin_Math_Floor },
            { Ceil,        TokenName.Builtin_Math_Ceil },
            { Round,       TokenName.Builtin_Math_Round },
            { Trunc,       TokenName.Builtin_Math_Trunc },
            { Remainder,   TokenName.Builtin_Math_Remainder },
            { Exp,         TokenName.Builtin_Math_Exp },
            { ExpM1,       TokenName.Builtin_Math_ExpM1 },
            { Erf,         TokenName.Builtin_Math_Erf },
            { ErfC,        TokenName.Builtin_Math_ErfC },
            { LGamma,      TokenName.Builtin_Math_LGamma },
            { TGamma,      TokenName.Builtin_Math_TGamma },
            { FMax,        TokenName.Builtin_Math_FMax },
            { FMin,        TokenName.Builtin_Math_FMin },
            { FDim,        TokenName.Builtin_Math_FDim },
            { CopySign,    TokenName.Builtin_Math_CopySign },
            { NextAfter,   TokenName.Builtin_Math_NextAfter },
            { Pow,         TokenName.Builtin_Math_Pow },
            { Epsilon,     TokenName.Builtin_Math_Epsilon },
            { Random,      TokenName.Builtin_Math_Random },
            { RotateLeft,  TokenName.Builtin_Math_RotateLeft },
            { RotateRight, TokenName.Builtin_Math_RotateRight },
            { Divisors,    TokenName.Builtin_Math_Divisors },
            { ListPrimes,  TokenName.Builtin_Math_ListPrimes },
            { NthPrime,    TokenName.Builtin_Math_NthPrime }
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
