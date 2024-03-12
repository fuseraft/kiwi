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

  std::unordered_set<SubTokenType> st_builtins = {
      SubTokenType::Builtin_FileIO_AppendText,
      SubTokenType::Builtin_FileIO_ChangeDirectory,
      SubTokenType::Builtin_FileIO_CopyFile,
      SubTokenType::Builtin_FileIO_CopyR,
      SubTokenType::Builtin_FileIO_CreateFile,
      SubTokenType::Builtin_FileIO_DeleteFile,
      SubTokenType::Builtin_FileIO_FileExists,
      SubTokenType::Builtin_FileIO_FileName,
      SubTokenType::Builtin_FileIO_FileSize,
      SubTokenType::Builtin_FileIO_GetCurrentDirectory,
      SubTokenType::Builtin_FileIO_GetFileAbsolutePath,
      SubTokenType::Builtin_FileIO_GetFileAttributes,
      SubTokenType::Builtin_FileIO_GetFileExtension,
      SubTokenType::Builtin_FileIO_GetFilePath,
      SubTokenType::Builtin_FileIO_Glob,
      SubTokenType::Builtin_FileIO_IsDirectory,
      SubTokenType::Builtin_FileIO_ListDirectory,
      SubTokenType::Builtin_FileIO_MakeDirectory,
      SubTokenType::Builtin_FileIO_MakeDirectoryP,
      SubTokenType::Builtin_FileIO_MoveFile,
      SubTokenType::Builtin_FileIO_ReadFile,
      SubTokenType::Builtin_FileIO_ReadLines,
      SubTokenType::Builtin_FileIO_RemoveDirectory,
      SubTokenType::Builtin_FileIO_RemoveDirectoryF,
      SubTokenType::Builtin_FileIO_TempDir,
      SubTokenType::Builtin_FileIO_WriteLine,
      SubTokenType::Builtin_FileIO_WriteText};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const SubTokenType& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} FileIOBuiltIns;

struct {
  const std::string Home = "__home__";

  std::unordered_set<std::string> builtins = {Home};
  std::unordered_set<SubTokenType> st_builtins = {
      SubTokenType::Builtin_Module_Home};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const SubTokenType& arg) {
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
  std::unordered_set<SubTokenType> st_builtins = {
      SubTokenType::Builtin_Time_AMPM,
      SubTokenType::Builtin_Time_Delay,
      SubTokenType::Builtin_Time_EpochMilliseconds,
      SubTokenType::Builtin_Time_Hour,
      SubTokenType::Builtin_Time_IsDST,
      SubTokenType::Builtin_Time_Minute,
      SubTokenType::Builtin_Time_Month,
      SubTokenType::Builtin_Time_MonthDay,
      SubTokenType::Builtin_Time_Second,
      SubTokenType::Builtin_Time_Ticks,
      SubTokenType::Builtin_Time_TicksToMilliseconds,
      SubTokenType::Builtin_Time_WeekDay,
      SubTokenType::Builtin_Time_Year,
      SubTokenType::Builtin_Time_YearDay};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const SubTokenType& arg) {
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
  const std::string ListPrimes = "__listprimes__";
  const std::string NthPrime = "__nthprime__";

  std::unordered_set<std::string> builtins = {
      Sin,        Tan,     Asin,     Acos,      Atan,   Atan2,    Sinh,
      Cosh,       Tanh,    Cos,      Log,       Log2,   Log10,    Log1P,
      Fmod,       Hypot,   IsFinite, IsInf,     IsNaN,  IsNormal, Sqrt,
      Cbrt,       Abs,     Floor,    Ceil,      Round,  Trunc,    Remainder,
      Exp,        ExpM1,   Erf,      ErfC,      LGamma, TGamma,   FMax,
      FMin,       FDim,    CopySign, NextAfter, Pow,    Epsilon,  Random,
      ListPrimes, NthPrime};

  std::unordered_set<SubTokenType> st_builtins = {
      SubTokenType::Builtin_Math_Abs,
      SubTokenType::Builtin_Math_Acos,
      SubTokenType::Builtin_Math_Asin,
      SubTokenType::Builtin_Math_Atan,
      SubTokenType::Builtin_Math_Atan2,
      SubTokenType::Builtin_Math_Cbrt,
      SubTokenType::Builtin_Math_Ceil,
      SubTokenType::Builtin_Math_CopySign,
      SubTokenType::Builtin_Math_Cos,
      SubTokenType::Builtin_Math_Cosh,
      SubTokenType::Builtin_Math_Epsilon,
      SubTokenType::Builtin_Math_Erf,
      SubTokenType::Builtin_Math_ErfC,
      SubTokenType::Builtin_Math_Exp,
      SubTokenType::Builtin_Math_ExpM1,
      SubTokenType::Builtin_Math_FDim,
      SubTokenType::Builtin_Math_Floor,
      SubTokenType::Builtin_Math_FMax,
      SubTokenType::Builtin_Math_FMin,
      SubTokenType::Builtin_Math_Fmod,
      SubTokenType::Builtin_Math_Hypot,
      SubTokenType::Builtin_Math_IsFinite,
      SubTokenType::Builtin_Math_IsInf,
      SubTokenType::Builtin_Math_IsNaN,
      SubTokenType::Builtin_Math_IsNormal,
      SubTokenType::Builtin_Math_LGamma,
      SubTokenType::Builtin_Math_Log,
      SubTokenType::Builtin_Math_Log10,
      SubTokenType::Builtin_Math_Log1P,
      SubTokenType::Builtin_Math_Log2,
      SubTokenType::Builtin_Math_NextAfter,
      SubTokenType::Builtin_Math_Pow,
      SubTokenType::Builtin_Math_Random,
      SubTokenType::Builtin_Math_Remainder,
      SubTokenType::Builtin_Math_Round,
      SubTokenType::Builtin_Math_Sin,
      SubTokenType::Builtin_Math_Sinh,
      SubTokenType::Builtin_Math_Sqrt,
      SubTokenType::Builtin_Math_Tan,
      SubTokenType::Builtin_Math_Tanh,
      SubTokenType::Builtin_Math_TGamma,
      SubTokenType::Builtin_Math_Trunc,
      SubTokenType::Builtin_Math_ListPrimes,
      SubTokenType::Builtin_Math_NthPrime};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const SubTokenType& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} MathBuiltins;

struct {
  const std::string Map = "map";
  const std::string None = "none";
  const std::string Reduce = "reduce";
  const std::string Select = "select";
  const std::string Sort = "sort";
  const std::string ToH = "to_h";

  std::unordered_set<std::string> builtins = {Map,    None, Reduce,
                                              Select, Sort, ToH};

  std::unordered_set<SubTokenType> st_builtins = {
      SubTokenType::Builtin_List_Map,    SubTokenType::Builtin_List_None,
      SubTokenType::Builtin_List_Reduce, SubTokenType::Builtin_List_Select,
      SubTokenType::Builtin_List_Sort,   SubTokenType::Builtin_List_ToH};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const SubTokenType& arg) {
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

  std::unordered_set<SubTokenType> st_builtins = {
      SubTokenType::Builtin_Env_GetEnvironmentVariable,
      SubTokenType::Builtin_Env_SetEnvironmentVariable};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const SubTokenType& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} EnvBuiltins;

struct {
  const std::string GetArgv = "__argv__";
  const std::string GetXarg = "__xarg__";

  std::unordered_set<std::string> builtins = {GetArgv, GetXarg};
  std::unordered_set<SubTokenType> st_builtins = {
      SubTokenType::Builtin_Argv_GetArgv, SubTokenType::Builtin_Argv_GetXarg};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const SubTokenType& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} ArgvBuiltins;

struct {
  const std::string EffectiveUserId = "__euid__";
  const std::string Exec = "__exec__";
  const std::string ExecOut = "__execout__";

  std::unordered_set<std::string> builtins = {EffectiveUserId, Exec, ExecOut};
  std::unordered_set<SubTokenType> st_builtins = {
      SubTokenType::Builtin_Sys_EffectiveUserId, SubTokenType::Builtin_Sys_Exec,
      SubTokenType::Builtin_Sys_ExecOut};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const SubTokenType& arg) {
    return st_builtins.find(arg) != st_builtins.end();
  }
} SysBuiltins;

struct {
  const std::string Input = "input";
  const std::string Silent = "silent";

  std::unordered_set<std::string> builtins = {Input, Silent};
  std::unordered_set<SubTokenType> st_builtins = {
      SubTokenType::Builtin_Console_Input,
      SubTokenType::Builtin_Console_Silent};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const SubTokenType& arg) {
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
  const std::string Replace = "replace";
  const std::string Reverse = "reverse";
  const std::string Contains = "contains";
  const std::string BeginsWith = "begins_with";
  const std::string EndsWith = "ends_with";
  const std::string IndexOf = "index_of";
  const std::string Upcase = "upcase";
  const std::string Downcase = "downcase";
  const std::string Keys = "keys";
  const std::string HasKey = "has_key";

  std::unordered_set<std::string> builtins = {
      Chars,     IsA,     Join,     Size,     ToD,        ToI,
      ToS,       Replace, Reverse,  Contains, BeginsWith, EndsWith,
      IndexOf,   Upcase,  Downcase, Keys,     Split,      LeftTrim,
      RightTrim, Trim,    Type,     HasKey};

  std::unordered_set<SubTokenType> st_builtins = {
      SubTokenType::Builtin_Kiwi_BeginsWith,
      SubTokenType::Builtin_Kiwi_BeginsWith,
      SubTokenType::Builtin_Kiwi_Chars,
      SubTokenType::Builtin_Kiwi_Contains,
      SubTokenType::Builtin_Kiwi_Downcase,
      SubTokenType::Builtin_Kiwi_EndsWith,
      SubTokenType::Builtin_Kiwi_HasKey,
      SubTokenType::Builtin_Kiwi_IndexOf,
      SubTokenType::Builtin_Kiwi_IsA,
      SubTokenType::Builtin_Kiwi_Join,
      SubTokenType::Builtin_Kiwi_Keys,
      SubTokenType::Builtin_Kiwi_LeftTrim,
      SubTokenType::Builtin_Kiwi_Replace,
      SubTokenType::Builtin_Kiwi_Reverse,
      SubTokenType::Builtin_Kiwi_RightTrim,
      SubTokenType::Builtin_Kiwi_Size,
      SubTokenType::Builtin_Kiwi_Split,
      SubTokenType::Builtin_Kiwi_ToD,
      SubTokenType::Builtin_Kiwi_ToI,
      SubTokenType::Builtin_Kiwi_ToS,
      SubTokenType::Builtin_Kiwi_Trim,
      SubTokenType::Builtin_Kiwi_Type,
      SubTokenType::Builtin_Kiwi_Upcase};

  bool is_builtin(const std::string& arg) {
    if (ListBuiltins.is_builtin(arg)) {
      return true;
    }
    return builtins.find(arg) != builtins.end();
  }

  bool is_builtin(const SubTokenType& arg) {
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

  bool is_builtin_method(const SubTokenType& arg) {
    return ConsoleBuiltins.is_builtin(arg) || EnvBuiltins.is_builtin(arg) ||
           ArgvBuiltins.is_builtin(arg) || TimeBuiltins.is_builtin(arg) ||
           FileIOBuiltIns.is_builtin(arg) || MathBuiltins.is_builtin(arg) ||
           ModuleBuiltins.is_builtin(arg) || SysBuiltins.is_builtin(arg);
  }
} KiwiBuiltins;

#endif
