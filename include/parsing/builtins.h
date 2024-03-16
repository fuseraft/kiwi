#ifndef KIWI_PARSING_BUILTINS_H
#define KIWI_PARSING_BUILTINS_H

#include <string>
#include <unordered_set>
#include "parsing/tokentype.h"

struct {
  // File operations
  const std::string AppendText = "__appendtext__";
  const std::string CopyFile = "__copyfile__";
  const std::string CopyR = "__copyr__";
  const std::string Combine = "__combine__";
  const std::string CreateFile = "__createfile__";
  const std::string DeleteFile = "__deletefile__";
  const std::string MoveFile = "__movefile__";
  const std::string ReadFile = "__readfile__";
  const std::string ReadLines = "__readlines__";
  const std::string WriteLine = "__writeline__";
  const std::string WriteText = "__writetext__";
  const std::string FileExists = "__isfile__";
  const std::string GetFileExtension = "__fileext__";
  const std::string FileName = "__filename__";
  const std::string FileSize = "__filesize__";
  const std::string GetFilePath = "__filepath__";
  const std::string GetFileAbsolutePath = "__fileabspath__";
  const std::string GetFileAttributes = "__fileattrs__";
  const std::string Glob = "__glob__";

  // Directory operations
  const std::string ListDirectory = "__listdir__";
  const std::string MakeDirectory = "__mkdir__";
  const std::string MakeDirectoryP = "__mkdirp__";
  const std::string RemoveDirectory = "__rmdir__";
  const std::string RemoveDirectoryF = "__rmdirf__";
  const std::string IsDirectory = "__isdir__";
  const std::string ChangeDirectory = "__chdir__";
  const std::string GetCurrentDirectory = "__cwd__";
  const std::string TempDir = "__tmpdir__";

  std::unordered_set<std::string> builtins = {AppendText,
                                              CopyFile,
                                              CopyR,
                                              Combine,
                                              CreateFile,
                                              DeleteFile,
                                              MoveFile,
                                              ReadFile,
                                              ReadLines,
                                              WriteText,
                                              WriteLine,
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
      KName::Builtin_FileIO_RemoveDirectory,
      KName::Builtin_FileIO_RemoveDirectoryF,
      KName::Builtin_FileIO_TempDir,
      KName::Builtin_FileIO_WriteLine,
      KName::Builtin_FileIO_WriteText};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} FileIOBuiltIns;

struct {
  const std::string Home = "__home__";

  std::unordered_set<std::string> builtins = {Home};
  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Module_Home};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} ModuleBuiltins;

struct {
  const std::string Hour = "__hour__";
  const std::string Minute = "__min__";
  const std::string Second = "__sec__";
  const std::string MonthDay = "__mday__";
  const std::string WeekDay = "__wday__";
  const std::string YearDay = "__yday__";
  const std::string Month = "__mon__";
  const std::string Year = "__year__";
  const std::string EpochMilliseconds = "__epochms__";
  const std::string Delay = "__delay__";
  const std::string IsDST = "__isdst__";
  const std::string Ticks = "__ticks__";
  const std::string TicksToMilliseconds = "__ticksms__";
  const std::string AMPM = "__ampm__";

  std::unordered_set<std::string> builtins = {
      Hour,    Minute, Second, MonthDay,           WeekDay,
      YearDay, Month,  Year,   EpochMilliseconds,  Delay,
      IsDST,   Ticks,  AMPM,   TicksToMilliseconds};
  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Time_AMPM,
      KName::Builtin_Time_Delay,
      KName::Builtin_Time_EpochMilliseconds,
      KName::Builtin_Time_Hour,
      KName::Builtin_Time_IsDST,
      KName::Builtin_Time_Minute,
      KName::Builtin_Time_Month,
      KName::Builtin_Time_MonthDay,
      KName::Builtin_Time_Second,
      KName::Builtin_Time_Ticks,
      KName::Builtin_Time_TicksToMilliseconds,
      KName::Builtin_Time_WeekDay,
      KName::Builtin_Time_Year,
      KName::Builtin_Time_YearDay};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} TimeBuiltins;

struct {
  const std::string Sin = "__sin__";
  const std::string Tan = "__tan__";
  const std::string Asin = "__asin__";
  const std::string Acos = "__acos__";
  const std::string Atan = "__atan__";
  const std::string Atan2 = "__atan2__";
  const std::string Sinh = "__sinh__";
  const std::string Cosh = "__cosh__";
  const std::string Tanh = "__tanh__";
  const std::string Cos = "__cos__";
  const std::string Log = "__log__";
  const std::string Log2 = "__log2__";
  const std::string Log10 = "__log10__";
  const std::string Log1P = "__log1p__";
  const std::string Fmod = "__fmod__";
  const std::string Hypot = "__hypot__";
  const std::string IsFinite = "__isfinite__";
  const std::string IsInf = "__isinf__";
  const std::string IsNaN = "__isnan__";
  const std::string IsNormal = "__isnormal__";
  const std::string Sqrt = "__sqrt__";
  const std::string Cbrt = "__cbrt__";
  const std::string Abs = "__abs__";
  const std::string Floor = "__floor__";
  const std::string Ceil = "__ceil__";
  const std::string Round = "__round__";
  const std::string Trunc = "__trunc__";
  const std::string Remainder = "__remainder__";
  const std::string Exp = "__exp__";
  const std::string ExpM1 = "__expm1__";
  const std::string Erf = "__erf__";
  const std::string ErfC = "__erfc__";
  const std::string LGamma = "__lgamma__";
  const std::string TGamma = "__tgamma__";
  const std::string FMax = "__fmax__";
  const std::string FMin = "__fmin__";
  const std::string FDim = "__fdim__";
  const std::string CopySign = "__copysign__";
  const std::string NextAfter = "__nextafter__";
  const std::string Pow = "__pow__";
  const std::string Epsilon = "__eps__";
  const std::string Random = "__random__";
  const std::string Divisors = "__divisors__";
  const std::string ListPrimes = "__listprimes__";
  const std::string NthPrime = "__nthprime__";

  std::unordered_set<std::string> builtins = {
      Sin,        Tan,     Asin,     Acos,      Atan,   Atan2,    Sinh,
      Cosh,       Tanh,    Cos,      Log,       Log2,   Log10,    Log1P,
      Fmod,       Hypot,   IsFinite, IsInf,     IsNaN,  IsNormal, Sqrt,
      Cbrt,       Abs,     Floor,    Ceil,      Round,  Trunc,    Remainder,
      Exp,        ExpM1,   Erf,      ErfC,      LGamma, TGamma,   FMax,
      FMin,       FDim,    CopySign, NextAfter, Pow,    Epsilon,  Random,
      ListPrimes, NthPrime, Divisors};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Math_Abs,
      KName::Builtin_Math_Acos,
      KName::Builtin_Math_Asin,
      KName::Builtin_Math_Atan,
      KName::Builtin_Math_Atan2,
      KName::Builtin_Math_Cbrt,
      KName::Builtin_Math_Ceil,
      KName::Builtin_Math_CopySign,
      KName::Builtin_Math_Cos,
      KName::Builtin_Math_Cosh,
      KName::Builtin_Math_Divisors,
      KName::Builtin_Math_Epsilon,
      KName::Builtin_Math_Erf,
      KName::Builtin_Math_ErfC,
      KName::Builtin_Math_Exp,
      KName::Builtin_Math_ExpM1,
      KName::Builtin_Math_FDim,
      KName::Builtin_Math_Floor,
      KName::Builtin_Math_FMax,
      KName::Builtin_Math_FMin,
      KName::Builtin_Math_Fmod,
      KName::Builtin_Math_Hypot,
      KName::Builtin_Math_IsFinite,
      KName::Builtin_Math_IsInf,
      KName::Builtin_Math_IsNaN,
      KName::Builtin_Math_IsNormal,
      KName::Builtin_Math_LGamma,
      KName::Builtin_Math_Log,
      KName::Builtin_Math_Log10,
      KName::Builtin_Math_Log1P,
      KName::Builtin_Math_Log2,
      KName::Builtin_Math_NextAfter,
      KName::Builtin_Math_Pow,
      KName::Builtin_Math_Random,
      KName::Builtin_Math_Remainder,
      KName::Builtin_Math_Round,
      KName::Builtin_Math_Sin,
      KName::Builtin_Math_Sinh,
      KName::Builtin_Math_Sqrt,
      KName::Builtin_Math_Tan,
      KName::Builtin_Math_Tanh,
      KName::Builtin_Math_TGamma,
      KName::Builtin_Math_Trunc,
      KName::Builtin_Math_ListPrimes,
      KName::Builtin_Math_NthPrime};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} MathBuiltins;

struct {
  const std::string Map = "map";
  const std::string None = "none";
  const std::string Reduce = "reduce";
  const std::string Select = "select";
  const std::string Sort = "sort";
  const std::string Sum = "sum";
  const std::string Min = "min";
  const std::string Max = "max";
  const std::string ToH = "to_h";

  std::unordered_set<std::string> builtins = {Map, None, Reduce, Select, Sort,
                                              Sum, Min,  Max,    ToH};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_List_Map,    KName::Builtin_List_None,
      KName::Builtin_List_Reduce, KName::Builtin_List_Select,
      KName::Builtin_List_Sort,   KName::Builtin_List_ToH,
      KName::Builtin_List_Sum,    KName::Builtin_List_Min,
      KName::Builtin_List_Max};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }

} ListBuiltins;

#ifdef EXPERIMENTAL_FEATURES
struct {
  const std::string Get = "__http_get__";
  const std::string Post = "__http_post__";
  const std::string Put = "__http_put__";
  const std::string Delete = "__http_delete__";
  const std::string Patch = "__http_patch__";
  const std::string Head = "__http_head__";
  const std::string Options = "__http_options__";

  std::unordered_set<std::string> builtins = {Get,   Post, Put,    Delete,
                                              Patch, Head, Options};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }
} HttpBuiltins;

struct {
  const std::string Connect = "__odbc_connect__";
  const std::string Exec = "__odbc_exec__";
  const std::string ExecSp = "__odbc_execsp__";
  const std::string IsConnected = "__odbc_isconnected__";
  const std::string BeginTransaction = "__odbc_begin_tx__";
  const std::string CommitTransaction = "__odbc_commit_tx__";
  const std::string RollbackTransaction = "__odbc_rollback_tx__";
  const std::string InTransaction = "__odbc_in_tx__";

  std::unordered_set<std::string> builtins = {Connect,
                                              Exec,
                                              ExecSp,
                                              IsConnected,
                                              BeginTransaction,
                                              CommitTransaction,
                                              RollbackTransaction,
                                              InTransaction};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }
} OdbcBuiltins;
#endif

struct {
  const std::string GetEnvironmentVariable = "__getenv__";
  const std::string SetEnvironmentVariable = "__setenv__";

  std::unordered_set<std::string> builtins = {GetEnvironmentVariable,
                                              SetEnvironmentVariable};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Env_GetEnvironmentVariable,
      KName::Builtin_Env_SetEnvironmentVariable};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} EnvBuiltins;

struct {
  const std::string GetArgv = "__argv__";
  const std::string GetXarg = "__xarg__";

  std::unordered_set<std::string> builtins = {GetArgv, GetXarg};
  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Argv_GetArgv, KName::Builtin_Argv_GetXarg};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} ArgvBuiltins;

struct {
  const std::string EffectiveUserId = "__euid__";
  const std::string Exec = "__exec__";
  const std::string ExecOut = "__execout__";

  std::unordered_set<std::string> builtins = {EffectiveUserId, Exec, ExecOut};
  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Sys_EffectiveUserId, KName::Builtin_Sys_Exec,
      KName::Builtin_Sys_ExecOut};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} SysBuiltins;

struct {
  const std::string Input = "input";
  const std::string Silent = "silent";

  std::unordered_set<std::string> builtins = {Input, Silent};
  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Console_Input,
      KName::Builtin_Console_Silent};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} ConsoleBuiltins;

struct {
  const std::string Chars = "chars";
  const std::string IsA = "is_a";
  const std::string Join = "join";
  const std::string Split = "split";
  const std::string LeftTrim = "ltrim";
  const std::string RightTrim = "rtrim";
  const std::string Trim = "trim";
  const std::string Size = "size";
  const std::string ToD = "to_d";
  const std::string ToI = "to_i";
  const std::string ToS = "to_s";
  const std::string Type = "type";
  const std::string Empty = "empty";
  const std::string Replace = "replace";
  const std::string Reverse = "reverse";
  const std::string Contains = "contains";
  const std::string BeginsWith = "begins_with";
  const std::string EndsWith = "ends_with";
  const std::string IndexOf = "index";
  const std::string LastIndexOf = "lastindex";
  const std::string Upcase = "upcase";
  const std::string Downcase = "downcase";
  const std::string Keys = "keys";
  const std::string HasKey = "has_key";

  std::unordered_set<std::string> builtins = {
      Chars,    Empty,     IsA,     Join,     Size,     ToD,
      ToI,      ToS,       Replace, Reverse,  Contains, BeginsWith,
      EndsWith, IndexOf,   LastIndexOf, Upcase,  Downcase, Keys,     Split,
      LeftTrim, RightTrim, Trim,    Type,     HasKey};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Kiwi_BeginsWith,
      KName::Builtin_Kiwi_BeginsWith,
      KName::Builtin_Kiwi_Empty,
      KName::Builtin_Kiwi_Chars,
      KName::Builtin_Kiwi_Contains,
      KName::Builtin_Kiwi_Downcase,
      KName::Builtin_Kiwi_EndsWith,
      KName::Builtin_Kiwi_HasKey,
      KName::Builtin_Kiwi_IndexOf,
      KName::Builtin_Kiwi_IsA,
      KName::Builtin_Kiwi_Join,
      KName::Builtin_Kiwi_Keys,
      KName::Builtin_Kiwi_LastIndexOf,
      KName::Builtin_Kiwi_LeftTrim,
      KName::Builtin_Kiwi_Replace,
      KName::Builtin_Kiwi_Reverse,
      KName::Builtin_Kiwi_RightTrim,
      KName::Builtin_Kiwi_Size,
      KName::Builtin_Kiwi_Split,
      KName::Builtin_Kiwi_ToD,
      KName::Builtin_Kiwi_ToI,
      KName::Builtin_Kiwi_ToS,
      KName::Builtin_Kiwi_Trim,
      KName::Builtin_Kiwi_Type,
      KName::Builtin_Kiwi_Upcase};

  bool is_builtin(const std::string& arg) {
    if (ListBuiltins.is_builtin(arg)) {
      return true;
    }
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end() ||
           ListBuiltins.is_builtin(arg);
  }

  bool is_builtin_method(const std::string& arg) {
#ifdef EXPERIMENTAL_FEATURES
    return ConsoleBuiltins.is_builtin(arg) || EnvBuiltins.is_builtin(arg) ||
           ArgvBuiltins.is_builtin(arg) || TimeBuiltins.is_builtin(arg) ||
           FileIOBuiltIns.is_builtin(arg) || MathBuiltins.is_builtin(arg) ||
           ModuleBuiltins.is_builtin(arg) || SysBuiltins.is_builtin(arg) ||
           HttpBuiltins.is_builtin(arg) || OdbcBuiltins.is_builtin(arg);
#endif
    return ConsoleBuiltins.is_builtin(arg) || EnvBuiltins.is_builtin(arg) ||
           ArgvBuiltins.is_builtin(arg) || TimeBuiltins.is_builtin(arg) ||
           FileIOBuiltIns.is_builtin(arg) || MathBuiltins.is_builtin(arg) ||
           ModuleBuiltins.is_builtin(arg) || SysBuiltins.is_builtin(arg);
  }

  bool is_builtin_method(const KName& arg) {
    return ConsoleBuiltins.is_builtin(arg) || EnvBuiltins.is_builtin(arg) ||
           ArgvBuiltins.is_builtin(arg) || TimeBuiltins.is_builtin(arg) ||
           FileIOBuiltIns.is_builtin(arg) || MathBuiltins.is_builtin(arg) ||
           ModuleBuiltins.is_builtin(arg) || SysBuiltins.is_builtin(arg);
  }
} KiwiBuiltins;

#endif
