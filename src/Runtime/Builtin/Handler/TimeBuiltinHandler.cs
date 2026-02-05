using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class TimeBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Time_Ticks => Ticks(token, args),
            TokenName.Builtin_Time_TicksToMilliseconds => TicksToMilliseconds(token, args),
            TokenName.Builtin_Time_AMPM => AMPM(token, args),
            TokenName.Builtin_Time_Hour => Hour(token, args),
            TokenName.Builtin_Time_Minute => Minute(token, args),
            TokenName.Builtin_Time_Second => Second(token, args),
            TokenName.Builtin_Time_Millisecond => Millisecond(token, args),
            TokenName.Builtin_Time_Year => Year(token, args),
            TokenName.Builtin_Time_Month => Month(token, args),
            TokenName.Builtin_Time_MonthDay => MonthDay(token, args),
            TokenName.Builtin_Time_YearDay => YearDay(token, args),
            TokenName.Builtin_Time_WeekDay => WeekDay(token, args),
            TokenName.Builtin_Time_IsDST => IsDST(token, args),
            TokenName.Builtin_Time_Now => Now(token, args),
            TokenName.Builtin_Time_AddDay => AddDay(token, args),
            TokenName.Builtin_Time_AddMonth => AddMonth(token, args),
            TokenName.Builtin_Time_AddYear => AddYear(token, args),
            TokenName.Builtin_Time_AddHour => AddHour(token, args),
            TokenName.Builtin_Time_AddMinute => AddMinute(token, args),
            TokenName.Builtin_Time_AddSecond => AddSecond(token, args),
            TokenName.Builtin_Time_AddMillisecond => AddMillisecond(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text)
        };
    }

    private static Value AddHour(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TimeBuiltin.AddHour, 2, args.Count);

        ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.AddHour, 0, args[0]);
        ParameterTypeMismatchError.ExpectNumber(token, TimeBuiltin.AddHour, 1, args[1]);

        return Value.CreateDate(args[0].GetDate().AddHours(args[1].GetNumber()));
    }

    private static Value AddMinute(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TimeBuiltin.AddMinute, 2, args.Count);

        ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.AddMinute, 0, args[0]);
        ParameterTypeMismatchError.ExpectNumber(token, TimeBuiltin.AddMinute, 1, args[1]);

        return Value.CreateDate(args[0].GetDate().AddMinutes(args[1].GetNumber()));
    }

    private static Value AddSecond(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TimeBuiltin.AddSecond, 2, args.Count);

        ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.AddSecond, 0, args[0]);
        ParameterTypeMismatchError.ExpectNumber(token, TimeBuiltin.AddSecond, 1, args[1]);

        return Value.CreateDate(args[0].GetDate().AddSeconds(args[1].GetNumber()));
    }

    private static Value AddMillisecond(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TimeBuiltin.AddMillisecond, 2, args.Count);

        ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.AddMillisecond, 0, args[0]);
        ParameterTypeMismatchError.ExpectNumber(token, TimeBuiltin.AddMillisecond, 1, args[1]);

        return Value.CreateDate(args[0].GetDate().AddMilliseconds(args[1].GetNumber()));
    }

    private static Value AddDay(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TimeBuiltin.AddDay, 2, args.Count);

        ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.AddDay, 0, args[0]);
        ParameterTypeMismatchError.ExpectNumber(token, TimeBuiltin.AddDay, 1, args[1]);

        return Value.CreateDate(args[0].GetDate().AddDays(args[1].GetNumber()));
    }

    private static Value AddMonth(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TimeBuiltin.AddMonth, 2, args.Count);

        ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.AddMonth, 0, args[0]);
        ParameterTypeMismatchError.ExpectNumber(token, TimeBuiltin.AddMonth, 1, args[1]);

        return Value.CreateDate(args[0].GetDate().AddMonths((int)args[1].GetNumber()));
    }

    private static Value AddYear(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TimeBuiltin.AddYear, 2, args.Count);

        ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.AddYear, 0, args[0]);
        ParameterTypeMismatchError.ExpectNumber(token, TimeBuiltin.AddYear, 1, args[1]);

        return Value.CreateDate(args[0].GetDate().AddYears((int)args[1].GetNumber()));
    }

    private static Value Now(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TimeBuiltin.Now, 0, args.Count);

        return Value.CreateDate(DateTime.Now);
    }

    private static Value IsDST(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.IsDST);
        }

        if (args.Count == 0)
        {
            return Value.CreateBoolean(DateTime.Now.IsDaylightSavingTime());
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.IsDST, 0, args[0]);
            return Value.CreateBoolean(args[0].GetDate().IsDaylightSavingTime());
        }

        throw new InvalidOperationError(token);
    }

    private static Value AMPM(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.AMPM);
        }

        if (args.Count == 0)
        {
            return Value.CreateString(DateTime.Now.ToString("tt"));
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.AMPM, 0, args[0]);
            return Value.CreateString(args[0].GetDate().ToString("tt"));
        }

        throw new InvalidOperationError(token);
    }

    private static Value Hour(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.Hour);
        }

        if (args.Count == 0)
        {
            return Value.CreateInteger(DateTime.Now.Hour);
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.Hour, 0, args[0]);
            return Value.CreateInteger(args[0].GetDate().Hour);
        }

        throw new InvalidOperationError(token);
    }

    private static Value Minute(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.Minute);
        }

        if (args.Count == 0)
        {
            return Value.CreateInteger(DateTime.Now.Minute);
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.Minute, 0, args[0]);
            return Value.CreateInteger(args[0].GetDate().Minute);
        }

        throw new InvalidOperationError(token);
    }

    private static Value Second(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.Second);
        }

        if (args.Count == 0)
        {
            return Value.CreateInteger(DateTime.Now.Second);
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.Second, 0, args[0]);
            return Value.CreateInteger(args[0].GetDate().Second);
        }

        throw new InvalidOperationError(token);
    }

    private static Value Millisecond(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.Millisecond);
        }

        if (args.Count == 0)
        {
            return Value.CreateInteger(DateTime.Now.Millisecond);
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.Millisecond, 0, args[0]);
            return Value.CreateInteger(args[0].GetDate().Millisecond);
        }

        throw new InvalidOperationError(token);
    }

    private static Value MonthDay(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.MonthDay);
        }

        if (args.Count == 0)
        {
            return Value.CreateInteger(DateTime.Now.Day);
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.MonthDay, 0, args[0]);
            return Value.CreateInteger(args[0].GetDate().Day);
        }

        throw new InvalidOperationError(token);
    }

    private static Value WeekDay(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.WeekDay);
        }

        if (args.Count == 0)
        {
            return Value.CreateInteger(DateTime.Now.DayOfWeek);
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.WeekDay, 0, args[0]);
            return Value.CreateInteger(args[0].GetDate().DayOfWeek);
        }

        throw new InvalidOperationError(token);
    }

    private static Value YearDay(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.YearDay);
        }

        if (args.Count == 0)
        {
            return Value.CreateInteger(DateTime.Now.DayOfYear);
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.YearDay, 0, args[0]);
            return Value.CreateInteger(args[0].GetDate().DayOfYear);
        }

        throw new InvalidOperationError(token);
    }

    private static Value Month(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.Month);
        }

        if (args.Count == 0)
        {
            return Value.CreateInteger(DateTime.Now.Month);
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.Month, 0, args[0]);
            return Value.CreateInteger(args[0].GetDate().Month);
        }

        throw new InvalidOperationError(token);
    }

    private static Value Year(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.Year);
        }

        if (args.Count == 0)
        {
            return Value.CreateInteger(DateTime.Now.Year);
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.Year, 0, args[0]);
            return Value.CreateInteger(args[0].GetDate().Year);
        }

        throw new InvalidOperationError(token);
    }

    private static Value Ticks(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, TimeBuiltin.Ticks);
        }

        if (args.Count == 0)
        {
            return Value.CreateFloat(GetTicks());
        }
        else if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectDate(token, TimeBuiltin.Ticks, 0, args[0]);
            return Value.CreateInteger(args[0].GetDate().Ticks);
        }

        throw new InvalidOperationError(token);
    }

    private static double GetTicks()
    {
        var rawTimestamp = System.Diagnostics.Stopwatch.GetTimestamp();        
        return rawTimestamp * (1e9 / System.Diagnostics.Stopwatch.Frequency);
    }

    private static Value TicksToMilliseconds(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TimeBuiltin.TicksToMilliseconds, 1, args.Count);

        ParameterTypeMismatchError.ExpectNumber(token, TimeBuiltin.TicksToMilliseconds, 0, args[0]);

        if (args[0].IsInteger())
        {
            return Value.CreateInteger(TimeSpan.FromTicks(args[0].GetInteger()).Milliseconds);
        }
        else if (args[0].IsFloat())
        {
            return Value.CreateFloat(args[0].GetFloat() / 1000000.0);
        }

        throw new InvalidOperationError(token, "Expected a number.");
    }
}