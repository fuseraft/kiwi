#ifndef KIWI_PARSING_BUILTINS_H
#define KIWI_PARSING_BUILTINS_H

#include <string>
#include <unordered_set>
#include "parsing/tokentype.h"
#include "typing/value.h"

struct {
  // File operations
  const k_string AppendText = "__appendtext__";
  const k_string CopyFile = "__copyfile__";
  const k_string CopyR = "__copyr__";
  const k_string Combine = "__combine__";
  const k_string CreateFile = "__createfile__";
  const k_string DeleteFile = "__deletefile__";
  const k_string MoveFile = "__movefile__";
  const k_string ReadFile = "__readfile__";
  const k_string ReadLines = "__readlines__";
  const k_string ReadBytes = "__readbytes__";
  const k_string WriteLine = "__writeline__";
  const k_string WriteText = "__writetext__";
  const k_string WriteBytes = "__writebytes__";
  const k_string FileExists = "__isfile__";
  const k_string GetFileExtension = "__fileext__";
  const k_string FileName = "__filename__";
  const k_string FileSize = "__filesize__";
  const k_string GetFilePath = "__filepath__";
  const k_string GetFileAbsolutePath = "__fileabspath__";
  const k_string GetFileAttributes = "__fileattrs__";
  const k_string Glob = "__glob__";

  // Directory operations
  const k_string ListDirectory = "__listdir__";
  const k_string MakeDirectory = "__mkdir__";
  const k_string MakeDirectoryP = "__mkdirp__";
  const k_string RemoveDirectory = "__rmdir__";
  const k_string RemoveDirectoryF = "__rmdirf__";
  const k_string IsDirectory = "__isdir__";
  const k_string ChangeDirectory = "__chdir__";
  const k_string GetCurrentDirectory = "__cwd__";
  const k_string TempDir = "__tmpdir__";

  std::unordered_set<k_string> builtins = {AppendText,
                                           CopyFile,
                                           CopyR,
                                           Combine,
                                           CreateFile,
                                           DeleteFile,
                                           MoveFile,
                                           ReadFile,
                                           ReadLines,
                                           ReadBytes,
                                           WriteText,
                                           WriteLine,
                                           WriteBytes,
                                           FileExists,
                                           FileName,
                                           FileSize,
                                           GetFileExtension,
                                           GetFilePath,
                                           GetFileAttributes,
                                           ListDirectory,
                                           MakeDirectory,
                                           MakeDirectoryP,
                                           RemoveDirectory,
                                           RemoveDirectoryF,
                                           IsDirectory,
                                           ChangeDirectory,
                                           GetCurrentDirectory,
                                           GetFileAbsolutePath,
                                           Glob,
                                           TempDir};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_FileIO_AppendText,
      KName::Builtin_FileIO_ChangeDirectory,
      KName::Builtin_FileIO_CopyFile,
      KName::Builtin_FileIO_CopyR,
      KName::Builtin_FileIO_Combine,
      KName::Builtin_FileIO_CreateFile,
      KName::Builtin_FileIO_DeleteFile,
      KName::Builtin_FileIO_FileExists,
      KName::Builtin_FileIO_FileName,
      KName::Builtin_FileIO_FileSize,
      KName::Builtin_FileIO_GetCurrentDirectory,
      KName::Builtin_FileIO_GetFileAbsolutePath,
      KName::Builtin_FileIO_GetFileAttributes,
      KName::Builtin_FileIO_GetFileExtension,
      KName::Builtin_FileIO_GetFilePath,
      KName::Builtin_FileIO_Glob,
      KName::Builtin_FileIO_IsDirectory,
      KName::Builtin_FileIO_ListDirectory,
      KName::Builtin_FileIO_MakeDirectory,
      KName::Builtin_FileIO_MakeDirectoryP,
      KName::Builtin_FileIO_MoveFile,
      KName::Builtin_FileIO_ReadFile,
      KName::Builtin_FileIO_ReadLines,
      KName::Builtin_FileIO_ReadBytes,
      KName::Builtin_FileIO_RemoveDirectory,
      KName::Builtin_FileIO_RemoveDirectoryF,
      KName::Builtin_FileIO_TempDir,
      KName::Builtin_FileIO_WriteBytes,
      KName::Builtin_FileIO_WriteLine,
      KName::Builtin_FileIO_WriteText};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} FileIOBuiltIns;

struct {
  const k_string Home = "__home__";

  std::unordered_set<k_string> builtins = {Home};
  std::unordered_set<KName> st_builtins = {KName::Builtin_Package_Home};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} PackageBuiltins;

struct {
  const k_string Hour = "__hour__";
  const k_string Minute = "__min__";
  const k_string Second = "__sec__";
  const k_string MonthDay = "__mday__";
  const k_string WeekDay = "__wday__";
  const k_string YearDay = "__yday__";
  const k_string Month = "__mon__";
  const k_string Year = "__year__";
  const k_string EpochMilliseconds = "__epochms__";
  const k_string Delay = "__delay__";
  const k_string IsDST = "__isdst__";
  const k_string Ticks = "__ticks__";
  const k_string TicksToMilliseconds = "__ticksms__";
  const k_string AMPM = "__ampm__";
  const k_string Timestamp = "__timestamp__";
  const k_string FormatDateTime = "__dtformat__";

  std::unordered_set<k_string> builtins = {Hour,
                                           Minute,
                                           Second,
                                           MonthDay,
                                           WeekDay,
                                           YearDay,
                                           Month,
                                           Year,
                                           EpochMilliseconds,
                                           Delay,
                                           IsDST,
                                           Ticks,
                                           AMPM,
                                           TicksToMilliseconds,
                                           Timestamp,
                                           FormatDateTime};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Time_AMPM,
      KName::Builtin_Time_Delay,
      KName::Builtin_Time_EpochMilliseconds,
      KName::Builtin_Time_FormatDateTime,
      KName::Builtin_Time_Hour,
      KName::Builtin_Time_IsDST,
      KName::Builtin_Time_Minute,
      KName::Builtin_Time_Month,
      KName::Builtin_Time_MonthDay,
      KName::Builtin_Time_Second,
      KName::Builtin_Time_Ticks,
      KName::Builtin_Time_TicksToMilliseconds,
      KName::Builtin_Time_Timestamp,
      KName::Builtin_Time_WeekDay,
      KName::Builtin_Time_Year,
      KName::Builtin_Time_YearDay};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} TimeBuiltins;

struct {
  const k_string Sin = "__sin__";
  const k_string Tan = "__tan__";
  const k_string Asin = "__asin__";
  const k_string Acos = "__acos__";
  const k_string Atan = "__atan__";
  const k_string Atan2 = "__atan2__";
  const k_string Sinh = "__sinh__";
  const k_string Cosh = "__cosh__";
  const k_string Tanh = "__tanh__";
  const k_string Cos = "__cos__";
  const k_string Log = "__log__";
  const k_string Log2 = "__log2__";
  const k_string Log10 = "__log10__";
  const k_string Log1P = "__log1p__";
  const k_string Fmod = "__fmod__";
  const k_string Hypot = "__hypot__";
  const k_string IsFinite = "__isfinite__";
  const k_string IsInf = "__isinf__";
  const k_string IsNaN = "__isnan__";
  const k_string IsNormal = "__isnormal__";
  const k_string Sqrt = "__sqrt__";
  const k_string Cbrt = "__cbrt__";
  const k_string Abs = "__abs__";
  const k_string Floor = "__floor__";
  const k_string Ceil = "__ceil__";
  const k_string Round = "__round__";
  const k_string Trunc = "__trunc__";
  const k_string Remainder = "__remainder__";
  const k_string Exp = "__exp__";
  const k_string ExpM1 = "__expm1__";
  const k_string Erf = "__erf__";
  const k_string ErfC = "__erfc__";
  const k_string LGamma = "__lgamma__";
  const k_string TGamma = "__tgamma__";
  const k_string FMax = "__fmax__";
  const k_string FMin = "__fmin__";
  const k_string FDim = "__fdim__";
  const k_string CopySign = "__copysign__";
  const k_string NextAfter = "__nextafter__";
  const k_string Pow = "__pow__";
  const k_string Epsilon = "__eps__";
  const k_string Random = "__random__";
  const k_string RotateLeft = "__rotl__";
  const k_string RotateRight = "__rotr__";
  const k_string Divisors = "__divisors__";
  const k_string ListPrimes = "__listprimes__";
  const k_string NthPrime = "__nthprime__";

  std::unordered_set<k_string> builtins = {
      Sin,        Tan,      Asin,      Acos,       Atan,       Atan2,
      Sinh,       Cosh,     Tanh,      Cos,        Log,        Log2,
      Log10,      Log1P,    Fmod,      Hypot,      IsFinite,   IsInf,
      IsNaN,      IsNormal, Sqrt,      Cbrt,       Abs,        Floor,
      Ceil,       Round,    Trunc,     Remainder,  Exp,        ExpM1,
      Erf,        ErfC,     LGamma,    TGamma,     FMax,       FMin,
      FDim,       CopySign, NextAfter, Pow,        Epsilon,    Random,
      ListPrimes, NthPrime, Divisors,  RotateLeft, RotateRight};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Math_Abs,        KName::Builtin_Math_Acos,
      KName::Builtin_Math_Asin,       KName::Builtin_Math_Atan,
      KName::Builtin_Math_Atan2,      KName::Builtin_Math_Cbrt,
      KName::Builtin_Math_Ceil,       KName::Builtin_Math_CopySign,
      KName::Builtin_Math_Cos,        KName::Builtin_Math_Cosh,
      KName::Builtin_Math_Divisors,   KName::Builtin_Math_Epsilon,
      KName::Builtin_Math_Erf,        KName::Builtin_Math_ErfC,
      KName::Builtin_Math_Exp,        KName::Builtin_Math_ExpM1,
      KName::Builtin_Math_FDim,       KName::Builtin_Math_Floor,
      KName::Builtin_Math_FMax,       KName::Builtin_Math_FMin,
      KName::Builtin_Math_Fmod,       KName::Builtin_Math_Hypot,
      KName::Builtin_Math_IsFinite,   KName::Builtin_Math_IsInf,
      KName::Builtin_Math_IsNaN,      KName::Builtin_Math_IsNormal,
      KName::Builtin_Math_LGamma,     KName::Builtin_Math_Log,
      KName::Builtin_Math_Log10,      KName::Builtin_Math_Log1P,
      KName::Builtin_Math_Log2,       KName::Builtin_Math_NextAfter,
      KName::Builtin_Math_Pow,        KName::Builtin_Math_Random,
      KName::Builtin_Math_Remainder,  KName::Builtin_Math_Round,
      KName::Builtin_Math_Sin,        KName::Builtin_Math_Sinh,
      KName::Builtin_Math_Sqrt,       KName::Builtin_Math_Tan,
      KName::Builtin_Math_Tanh,       KName::Builtin_Math_TGamma,
      KName::Builtin_Math_Trunc,      KName::Builtin_Math_ListPrimes,
      KName::Builtin_Math_NthPrime,   KName::Builtin_Math_RotateLeft,
      KName::Builtin_Math_RotateRight};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} MathBuiltins;

struct {
  const k_string Deserialize = "deserialize";
  const k_string Serialize = "serialize";

  std::unordered_set<k_string> builtins = {Deserialize, Serialize};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Serializer_Deserialize,
      KName::Builtin_Serializer_Serialize};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} SerializerBuiltins;

struct {
  const k_string RInspect = "__rinspect__";
  const k_string RList = "__rlist__";

  std::unordered_set<k_string> builtins = {RInspect, RList};

  std::unordered_set<KName> st_builtins = {KName::Builtin_Reflector_RInspect,
                                           KName::Builtin_Reflector_RList};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} ReflectorBuiltins;

struct {
  const k_string Each = "each";
  const k_string Map = "map";
  const k_string None = "none";
  const k_string Reduce = "reduce";
  const k_string Select = "select";
  const k_string Sort = "sort";
  const k_string Sum = "sum";
  const k_string Min = "min";
  const k_string Max = "max";
  const k_string ToH = "to_hash";

  std::unordered_set<k_string> builtins = {Each, Map, None, Reduce, Select,
                                           Sort, Sum, Min,  Max,    ToH};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_List_Each,   KName::Builtin_List_Map,
      KName::Builtin_List_None,   KName::Builtin_List_Reduce,
      KName::Builtin_List_Select, KName::Builtin_List_Sort,
      KName::Builtin_List_ToH,    KName::Builtin_List_Sum,
      KName::Builtin_List_Min,    KName::Builtin_List_Max};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }

} ListBuiltins;

struct {
  const k_string Get = "__webs_get__";
  const k_string Post = "__webs_post__";
  const k_string Listen = "__webs_listen__";
  const k_string Host = "__webs_host__";
  const k_string Port = "__webs_port__";
  const k_string Public = "__webs_public__";

  std::unordered_set<k_string> builtins = {Get,  Post, Listen,
                                           Host, Port, Public};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_WebServer_Get,    KName::Builtin_WebServer_Post,
      KName::Builtin_WebServer_Listen, KName::Builtin_WebServer_Host,
      KName::Builtin_WebServer_Port,   KName::Builtin_WebServer_Public};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} WebServerBuiltins;

struct {
  const k_string FilePath = "__log_filepath__";
  const k_string Mode = "__log_mode__";
  const k_string EntryFormat = "__log_format__";
  const k_string TimestampFormat = "__log_tsformat__";
  const k_string Level = "__log_level__";
  const k_string Debug = "__log_debug__";
  const k_string Warn = "__log_warn__";
  const k_string Info = "__log_info__";
  const k_string Error = "__log_error__";

  std::unordered_set<k_string> builtins = {
      FilePath, Mode, EntryFormat, TimestampFormat, Level, Debug,
      Warn,     Info, Error};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Logging_FilePath,
      KName::Builtin_Logging_Mode,
      KName::Builtin_Logging_EntryFormat,
      KName::Builtin_Logging_TimestampFormat,
      KName::Builtin_Logging_Level,
      KName::Builtin_Logging_Debug,
      KName::Builtin_Logging_Warn,
      KName::Builtin_Logging_Info,
      KName::Builtin_Logging_Error};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} LoggingBuiltins;

struct {
  const k_string Get = "__webc_get__";
  const k_string Post = "__webc_post__";
  const k_string Put = "__webc_put__";
  const k_string Delete = "__webc_delete__";
  const k_string Patch = "__webc_patch__";
  const k_string Head = "__webc_head__";
  const k_string Options = "__webc_options__";

  std::unordered_set<k_string> builtins = {Get,   Post, Put,    Delete,
                                           Patch, Head, Options};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_WebClient_Delete, KName::Builtin_WebClient_Get,
      KName::Builtin_WebClient_Head,   KName::Builtin_WebClient_Options,
      KName::Builtin_WebClient_Patch,  KName::Builtin_WebClient_Post,
      KName::Builtin_WebClient_Put};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} HttpBuiltins;

struct {
  const k_string GetEnvironmentVariable = "__getenv__";
  const k_string SetEnvironmentVariable = "__setenv__";
  const k_string UnsetEnvironmentVariable = "__unsetenv__";
  const k_string Kiwi = "__kiwi__";
  const k_string KiwiLib = "__kiwilib__";

  std::unordered_set<k_string> builtins = {
      GetEnvironmentVariable, SetEnvironmentVariable, UnsetEnvironmentVariable,
      Kiwi, KiwiLib};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Env_GetEnvironmentVariable,
      KName::Builtin_Env_SetEnvironmentVariable,
      KName::Builtin_Env_UnsetEnvironmentVariable, KName::Builtin_Env_Kiwi,
      KName::Builtin_Env_KiwiLib};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} EnvBuiltins;

struct {
  const k_string GetArgv = "__argv__";
  const k_string GetXarg = "__xarg__";

  std::unordered_set<k_string> builtins = {GetArgv, GetXarg};
  std::unordered_set<KName> st_builtins = {KName::Builtin_Argv_GetArgv,
                                           KName::Builtin_Argv_GetXarg};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} ArgvBuiltins;

struct {
  const k_string EffectiveUserId = "__euid__";
  const k_string Exec = "__exec__";
  const k_string ExecOut = "__execout__";

  std::unordered_set<k_string> builtins = {EffectiveUserId, Exec, ExecOut};
  std::unordered_set<KName> st_builtins = {KName::Builtin_Sys_EffectiveUserId,
                                           KName::Builtin_Sys_Exec,
                                           KName::Builtin_Sys_ExecOut};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} SysBuiltins;

struct {
  const k_string Base64Encode = "__base64encode__";
  const k_string Base64Decode = "__base64decode__";
  const k_string UrlEncode = "__urlencode__";
  const k_string UrlDecode = "__urldecode__";

  std::unordered_set<k_string> builtins = {Base64Encode, Base64Decode,
                                           UrlEncode, UrlDecode};
  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Encoder_Base64Encode, KName::Builtin_Encoder_Base64Decode,
      KName::Builtin_Encoder_UrlEncode, KName::Builtin_Encoder_UrlDecode};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} EncoderBuiltins;

struct {
  const k_string Input = "input";
  const k_string Silent = "silent";

  std::unordered_set<k_string> builtins = {Input, Silent};
  std::unordered_set<KName> st_builtins = {KName::Builtin_Console_Input,
                                           KName::Builtin_Console_Silent};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} ConsoleBuiltins;

struct {
  const k_string Chars = "chars";
  const k_string IsA = "is_a";
  const k_string Join = "join";
  const k_string Split = "split";
  const k_string Substring = "substring";
  const k_string LeftTrim = "ltrim";
  const k_string RightTrim = "rtrim";
  const k_string Trim = "trim";
  const k_string Size = "size";
  const k_string ToD = "to_double";
  const k_string ToI = "to_int";
  const k_string ToS = "to_string";
  const k_string ToBytes = "to_bytes";
  const k_string ToHex = "to_hex";
  const k_string Type = "type";
  const k_string Empty = "empty";
  const k_string Replace = "replace";
  const k_string Reverse = "reverse";
  const k_string Contains = "contains";
  const k_string BeginsWith = "begins_with";
  const k_string EndsWith = "ends_with";
  const k_string IndexOf = "index";
  const k_string LastIndexOf = "lastindex";
  const k_string Uppercase = "uppercase";
  const k_string Lowercase = "lowercase";
  const k_string Keys = "keys";
  const k_string HasKey = "has_key";
  const k_string Members = "members";
  const k_string Push = "push";
  const k_string Pop = "pop";
  const k_string Enqueue = "enqueue";
  const k_string Dequeue = "dequeue";
  const k_string Shift = "shift";
  const k_string Unshift = "unshift";
  const k_string Clear = "clear";
  const k_string Remove = "remove";
  const k_string RemoveAt = "remove_at";
  const k_string Rotate = "rotate";
  const k_string Insert = "insert";
  const k_string Slice = "slice";
  const k_string Concat = "concat";
  const k_string Unique = "unique";
  const k_string Count = "count";
  const k_string Flatten = "flatten";
  const k_string Zip = "zip";
  const k_string Merge = "merge";
  const k_string Values = "values";
  const k_string Clone = "clone";
  const k_string Pretty = "pretty";
  const k_string Find = "find";
  const k_string Match = "match";
  const k_string Matches = "matches";
  const k_string MatchesAll = "matches_all";
  const k_string Scan = "scan";
  const k_string Set = "set";
  const k_string Swap = "swap";
  const k_string Get = "get";
  const k_string First = "first";
  const k_string Last = "last";
  const k_string Truthy = "truthy";

  std::unordered_set<k_string> builtins = {
      Chars,     Empty,      IsA,       Join,     Size,        ToBytes,
      ToHex,     ToD,        ToI,       ToS,      Replace,     Reverse,
      Contains,  BeginsWith, EndsWith,  IndexOf,  LastIndexOf, Uppercase,
      Lowercase, Keys,       Split,     LeftTrim, RightTrim,   Trim,
      Type,      HasKey,     Members,   Push,     Pop,         Enqueue,
      Dequeue,   Clear,      Substring, Shift,    Unshift,     Remove,
      RemoveAt,  Rotate,     Insert,    Slice,    Concat,      Unique,
      Count,     Flatten,    Zip,       Merge,    Values,      Clone,
      Pretty,    Find,       Match,     Matches,  MatchesAll,  Scan,
      Set,       Get,        Swap,      First,    Last,        Truthy};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Kiwi_BeginsWith,  KName::Builtin_Kiwi_BeginsWith,
      KName::Builtin_Kiwi_Empty,       KName::Builtin_Kiwi_Chars,
      KName::Builtin_Kiwi_Contains,    KName::Builtin_Kiwi_Lowercase,
      KName::Builtin_Kiwi_EndsWith,    KName::Builtin_Kiwi_HasKey,
      KName::Builtin_Kiwi_IndexOf,     KName::Builtin_Kiwi_IsA,
      KName::Builtin_Kiwi_Join,        KName::Builtin_Kiwi_Keys,
      KName::Builtin_Kiwi_LastIndexOf, KName::Builtin_Kiwi_LeftTrim,
      KName::Builtin_Kiwi_Replace,     KName::Builtin_Kiwi_Reverse,
      KName::Builtin_Kiwi_RightTrim,   KName::Builtin_Kiwi_Size,
      KName::Builtin_Kiwi_Split,       KName::Builtin_Kiwi_ToBytes,
      KName::Builtin_Kiwi_ToHex,       KName::Builtin_Kiwi_ToD,
      KName::Builtin_Kiwi_ToI,         KName::Builtin_Kiwi_ToS,
      KName::Builtin_Kiwi_Trim,        KName::Builtin_Kiwi_Type,
      KName::Builtin_Kiwi_Uppercase,   KName::Builtin_Kiwi_Members,
      KName::Builtin_Kiwi_Push,        KName::Builtin_Kiwi_Pop,
      KName::Builtin_Kiwi_Enqueue,     KName::Builtin_Kiwi_Dequeue,
      KName::Builtin_Kiwi_Shift,       KName::Builtin_Kiwi_Unshift,
      KName::Builtin_Kiwi_Remove,      KName::Builtin_Kiwi_RemoveAt,
      KName::Builtin_Kiwi_Rotate,      KName::Builtin_Kiwi_Insert,
      KName::Builtin_Kiwi_Slice,       KName::Builtin_Kiwi_Concat,
      KName::Builtin_Kiwi_Unique,      KName::Builtin_Kiwi_Count,
      KName::Builtin_Kiwi_Flatten,     KName::Builtin_Kiwi_Zip,
      KName::Builtin_Kiwi_Clear,       KName::Builtin_Kiwi_Substring,
      KName::Builtin_Kiwi_Merge,       KName::Builtin_Kiwi_Values,
      KName::Builtin_Kiwi_Clone,       KName::Builtin_Kiwi_Pretty,
      KName::Builtin_Kiwi_Find,        KName::Builtin_Kiwi_Match,
      KName::Builtin_Kiwi_Matches,     KName::Builtin_Kiwi_MatchesAll,
      KName::Builtin_Kiwi_Scan,        KName::Builtin_Kiwi_Set,
      KName::Builtin_Kiwi_Get,         KName::Builtin_Kiwi_Swap,
      KName::Builtin_Kiwi_First,       KName::Builtin_Kiwi_Last,
      KName::Builtin_Kiwi_Truthy};

  bool is_builtin(const k_string& arg) {
    if (ListBuiltins.is_builtin(arg)) {
      return true;
    }
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end() ||
           ListBuiltins.is_builtin(arg);
  }

  bool is_builtin_method(const k_string& arg) {
    return ConsoleBuiltins.is_builtin(arg) || EnvBuiltins.is_builtin(arg) ||
           ArgvBuiltins.is_builtin(arg) || TimeBuiltins.is_builtin(arg) ||
           FileIOBuiltIns.is_builtin(arg) || MathBuiltins.is_builtin(arg) ||
           PackageBuiltins.is_builtin(arg) || SysBuiltins.is_builtin(arg) ||
           HttpBuiltins.is_builtin(arg) || WebServerBuiltins.is_builtin(arg) ||
           LoggingBuiltins.is_builtin(arg) || EncoderBuiltins.is_builtin(arg) ||
           SerializerBuiltins.is_builtin(arg) ||
           ReflectorBuiltins.is_builtin(arg);
  }

  bool is_builtin_method(const KName& arg) {
    return ConsoleBuiltins.is_builtin(arg) || EnvBuiltins.is_builtin(arg) ||
           ArgvBuiltins.is_builtin(arg) || TimeBuiltins.is_builtin(arg) ||
           FileIOBuiltIns.is_builtin(arg) || MathBuiltins.is_builtin(arg) ||
           PackageBuiltins.is_builtin(arg) || SysBuiltins.is_builtin(arg) ||
           HttpBuiltins.is_builtin(arg) || WebServerBuiltins.is_builtin(arg) ||
           LoggingBuiltins.is_builtin(arg) || EncoderBuiltins.is_builtin(arg) ||
           SerializerBuiltins.is_builtin(arg) ||
           ReflectorBuiltins.is_builtin(arg);
  }
} KiwiBuiltins;

#endif
