#ifndef ASTRAL_PARSING_BUILTINS_H
#define ASTRAL_PARSING_BUILTINS_H

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
  std::unordered_set<KName> st_builtins = {KName::Builtin_Module_Home};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} ModuleBuiltins;

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

  std::unordered_set<k_string> builtins = {
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
  const k_string Divisors = "__divisors__";
  const k_string ListPrimes = "__listprimes__";
  const k_string NthPrime = "__nthprime__";

  std::unordered_set<k_string> builtins = {
      Sin,        Tan,      Asin,     Acos,      Atan,   Atan2,    Sinh,
      Cosh,       Tanh,     Cos,      Log,       Log2,   Log10,    Log1P,
      Fmod,       Hypot,    IsFinite, IsInf,     IsNaN,  IsNormal, Sqrt,
      Cbrt,       Abs,      Floor,    Ceil,      Round,  Trunc,    Remainder,
      Exp,        ExpM1,    Erf,      ErfC,      LGamma, TGamma,   FMax,
      FMin,       FDim,     CopySign, NextAfter, Pow,    Epsilon,  Random,
      ListPrimes, NthPrime, Divisors};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Math_Abs,       KName::Builtin_Math_Acos,
      KName::Builtin_Math_Asin,      KName::Builtin_Math_Atan,
      KName::Builtin_Math_Atan2,     KName::Builtin_Math_Cbrt,
      KName::Builtin_Math_Ceil,      KName::Builtin_Math_CopySign,
      KName::Builtin_Math_Cos,       KName::Builtin_Math_Cosh,
      KName::Builtin_Math_Divisors,  KName::Builtin_Math_Epsilon,
      KName::Builtin_Math_Erf,       KName::Builtin_Math_ErfC,
      KName::Builtin_Math_Exp,       KName::Builtin_Math_ExpM1,
      KName::Builtin_Math_FDim,      KName::Builtin_Math_Floor,
      KName::Builtin_Math_FMax,      KName::Builtin_Math_FMin,
      KName::Builtin_Math_Fmod,      KName::Builtin_Math_Hypot,
      KName::Builtin_Math_IsFinite,  KName::Builtin_Math_IsInf,
      KName::Builtin_Math_IsNaN,     KName::Builtin_Math_IsNormal,
      KName::Builtin_Math_LGamma,    KName::Builtin_Math_Log,
      KName::Builtin_Math_Log10,     KName::Builtin_Math_Log1P,
      KName::Builtin_Math_Log2,      KName::Builtin_Math_NextAfter,
      KName::Builtin_Math_Pow,       KName::Builtin_Math_Random,
      KName::Builtin_Math_Remainder, KName::Builtin_Math_Round,
      KName::Builtin_Math_Sin,       KName::Builtin_Math_Sinh,
      KName::Builtin_Math_Sqrt,      KName::Builtin_Math_Tan,
      KName::Builtin_Math_Tanh,      KName::Builtin_Math_TGamma,
      KName::Builtin_Math_Trunc,     KName::Builtin_Math_ListPrimes,
      KName::Builtin_Math_NthPrime};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} MathBuiltins;

struct {
  const k_string Map = "map";
  const k_string None = "none";
  const k_string Reduce = "reduce";
  const k_string Select = "select";
  const k_string Sort = "sort";
  const k_string Sum = "sum";
  const k_string Min = "min";
  const k_string Max = "max";
  const k_string ToH = "to_hash";

  std::unordered_set<k_string> builtins = {Map, None, Reduce, Select, Sort,
                                           Sum, Min,  Max,    ToH};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_List_Map,    KName::Builtin_List_None,
      KName::Builtin_List_Reduce, KName::Builtin_List_Select,
      KName::Builtin_List_Sort,   KName::Builtin_List_ToH,
      KName::Builtin_List_Sum,    KName::Builtin_List_Min,
      KName::Builtin_List_Max};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const KName& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }

} ListBuiltins;

struct {
  const k_string Get = "__webserver_get__";
  const k_string Post = "__webserver_post__";
  const k_string Listen = "__webserver_listen__";
  const k_string Host = "__webserver_host__";
  const k_string Port = "__webserver_port__";
  const k_string Public = "__webserver_public__";

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

#ifdef EXPERIMENTAL_FEATURES
struct {
  const k_string Get = "__http_get__";
  const k_string Post = "__http_post__";
  const k_string Put = "__http_put__";
  const k_string Delete = "__http_delete__";
  const k_string Patch = "__http_patch__";
  const k_string Head = "__http_head__";
  const k_string Options = "__http_options__";

  std::unordered_set<k_string> builtins = {Get,   Post, Put,    Delete,
                                           Patch, Head, Options};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }
} HttpBuiltins;

struct {
  const k_string Connect = "__odbc_connect__";
  const k_string Exec = "__odbc_exec__";
  const k_string ExecSp = "__odbc_execsp__";
  const k_string IsConnected = "__odbc_isconnected__";
  const k_string BeginTransaction = "__odbc_begin_tx__";
  const k_string CommitTransaction = "__odbc_commit_tx__";
  const k_string RollbackTransaction = "__odbc_rollback_tx__";
  const k_string InTransaction = "__odbc_in_tx__";

  std::unordered_set<k_string> builtins = {Connect,
                                           Exec,
                                           ExecSp,
                                           IsConnected,
                                           BeginTransaction,
                                           CommitTransaction,
                                           RollbackTransaction,
                                           InTransaction};

  bool is_builtin(const k_string& arg) {
    return builtins.find(arg) != builtins.end();
  }
} OdbcBuiltins;
#endif

struct {
  const k_string GetEnvironmentVariable = "__getenv__";
  const k_string SetEnvironmentVariable = "__setenv__";

  std::unordered_set<k_string> builtins = {GetEnvironmentVariable,
                                           SetEnvironmentVariable};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Env_GetEnvironmentVariable,
      KName::Builtin_Env_SetEnvironmentVariable};

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
  const k_string Upcase = "upcase";
  const k_string Downcase = "downcase";
  const k_string Keys = "keys";
  const k_string HasKey = "has_key";

  std::unordered_set<k_string> builtins = {
      Chars,    Empty,     IsA,         Join,    Size,     ToBytes,  ToHex,
      ToD,      ToI,       ToS,         Replace, Reverse,  Contains, BeginsWith,
      EndsWith, IndexOf,   LastIndexOf, Upcase,  Downcase, Keys,     Split,
      LeftTrim, RightTrim, Trim,        Type,    HasKey};

  std::unordered_set<KName> st_builtins = {
      KName::Builtin_Astral_BeginsWith,  KName::Builtin_Astral_BeginsWith,
      KName::Builtin_Astral_Empty,       KName::Builtin_Astral_Chars,
      KName::Builtin_Astral_Contains,    KName::Builtin_Astral_Downcase,
      KName::Builtin_Astral_EndsWith,    KName::Builtin_Astral_HasKey,
      KName::Builtin_Astral_IndexOf,     KName::Builtin_Astral_IsA,
      KName::Builtin_Astral_Join,        KName::Builtin_Astral_Keys,
      KName::Builtin_Astral_LastIndexOf, KName::Builtin_Astral_LeftTrim,
      KName::Builtin_Astral_Replace,     KName::Builtin_Astral_Reverse,
      KName::Builtin_Astral_RightTrim,   KName::Builtin_Astral_Size,
      KName::Builtin_Astral_Split,       KName::Builtin_Astral_ToBytes,
      KName::Builtin_Astral_ToHex,       KName::Builtin_Astral_ToD,
      KName::Builtin_Astral_ToI,         KName::Builtin_Astral_ToS,
      KName::Builtin_Astral_Trim,        KName::Builtin_Astral_Type,
      KName::Builtin_Astral_Upcase};

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
           ModuleBuiltins.is_builtin(arg) || SysBuiltins.is_builtin(arg) ||
           WebServerBuiltins.is_builtin(arg);
  }

  bool is_builtin_method(const KName& arg) {
    return ConsoleBuiltins.is_builtin(arg) || EnvBuiltins.is_builtin(arg) ||
           ArgvBuiltins.is_builtin(arg) || TimeBuiltins.is_builtin(arg) ||
           FileIOBuiltIns.is_builtin(arg) || MathBuiltins.is_builtin(arg) ||
           ModuleBuiltins.is_builtin(arg) || SysBuiltins.is_builtin(arg) ||
           WebServerBuiltins.is_builtin(arg);
  }
} AstralBuiltins;

#endif
