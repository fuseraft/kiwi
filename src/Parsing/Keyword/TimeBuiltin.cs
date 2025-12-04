namespace kiwi.Parsing.Keyword;

public static class TimeBuiltin
{
    public const string Hour = "__time_hour__";
    public const string Minute = "__time_minute__";
    public const string Second = "__time_second__";
    public const string Millisecond = "__time_millisecond__";
    public const string MonthDay = "__time_monthday__";
    public const string WeekDay = "__time_weekday__";
    public const string YearDay = "__time_yearday__";
    public const string Month = "__time_month__";
    public const string Year = "__time_year__";
    public const string IsDST = "__time_isdst__";
    public const string Ticks = "__time_ticks__";
    public const string TicksToMilliseconds = "__time_ticksms__";
    public const string AMPM = "__time_ampm__";
    public const string Now = "__time_now__";
    public const string AddDay = "__time_dayadd__";
    public const string AddMonth = "__time_monthadd__";
    public const string AddYear = "__time_yearadd__";
    public const string AddHour = "__time_houradd__";
    public const string AddMinute = "__time_minuteadd__";
    public const string AddSecond = "__time_secadd__";
    public const string AddMillisecond = "__time_millisecadd__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map
        = new Dictionary<string, TokenName>
        {
            { Now,                 TokenName.Builtin_Time_Now },
            { Hour,                TokenName.Builtin_Time_Hour },
            { Minute,              TokenName.Builtin_Time_Minute },
            { Second,              TokenName.Builtin_Time_Second },
            { Millisecond,         TokenName.Builtin_Time_Millisecond },
            { MonthDay,            TokenName.Builtin_Time_MonthDay },
            { WeekDay,             TokenName.Builtin_Time_WeekDay },
            { YearDay,             TokenName.Builtin_Time_YearDay },
            { Month,               TokenName.Builtin_Time_Month },
            { Year,                TokenName.Builtin_Time_Year },
            { IsDST,               TokenName.Builtin_Time_IsDST },
            { Ticks,               TokenName.Builtin_Time_Ticks },
            { TicksToMilliseconds, TokenName.Builtin_Time_TicksToMilliseconds },
            { AddDay,              TokenName.Builtin_Time_AddDay },
            { AddMonth,            TokenName.Builtin_Time_AddMonth },
            { AddYear,             TokenName.Builtin_Time_AddYear },
            { AddHour,             TokenName.Builtin_Time_AddHour },
            { AddMinute,           TokenName.Builtin_Time_AddMinute },
            { AddSecond,           TokenName.Builtin_Time_AddSecond },
            { AddMillisecond,      TokenName.Builtin_Time_AddMillisecond },
            { AMPM,                TokenName.Builtin_Time_AMPM },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;

    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}
