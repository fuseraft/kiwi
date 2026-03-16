using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class MathBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Math_Abs         => Abs(token, args),
            TokenName.Builtin_Math_Acos        => FloatUnary(token, MathBuiltin.Acos, args, Math.Acos),
            TokenName.Builtin_Math_Asin        => FloatUnary(token, MathBuiltin.Asin, args, Math.Asin),
            TokenName.Builtin_Math_Atan        => FloatUnary(token, MathBuiltin.Atan, args, Math.Atan),
            TokenName.Builtin_Math_Atan2       => FloatBinary(token, MathBuiltin.Atan2, args, Math.Atan2),
            TokenName.Builtin_Math_Cbrt        => FloatUnary(token, MathBuiltin.Cbrt, args, Math.Cbrt),
            TokenName.Builtin_Math_Ceil        => FloatUnary(token, MathBuiltin.Ceil, args, Math.Ceiling),
            TokenName.Builtin_Math_CopySign    => FloatBinary(token, MathBuiltin.CopySign, args, Math.CopySign),
            TokenName.Builtin_Math_Cos         => FloatUnary(token, MathBuiltin.Cos, args, Math.Cos),
            TokenName.Builtin_Math_Cosh        => FloatUnary(token, MathBuiltin.Cosh, args, Math.Cosh),
            TokenName.Builtin_Math_Divisors    => Divisors(token, args),
            TokenName.Builtin_Math_Epsilon     => Eps(token, args),
            TokenName.Builtin_Math_Erf         => FloatUnary(token, MathBuiltin.Erf, args, ErfApprox),
            TokenName.Builtin_Math_ErfC        => FloatUnary(token, MathBuiltin.ErfC, args, x => 1.0 - ErfApprox(x)),
            TokenName.Builtin_Math_Exp         => FloatUnary(token, MathBuiltin.Exp, args, Math.Exp),
            TokenName.Builtin_Math_ExpM1       => FloatUnary(token, MathBuiltin.ExpM1, args, x => Math.Exp(x) - 1.0),
            TokenName.Builtin_Math_FDim        => FloatBinary(token, MathBuiltin.FDim, args, FDimImpl),
            TokenName.Builtin_Math_Floor       => FloatUnary(token, MathBuiltin.Floor, args, Math.Floor),
            TokenName.Builtin_Math_FMax        => FloatBinary(token, MathBuiltin.FMax, args, FMaxImpl),
            TokenName.Builtin_Math_FMin        => FloatBinary(token, MathBuiltin.FMin, args, FMinImpl),
            TokenName.Builtin_Math_Fmod        => FloatBinary(token, MathBuiltin.Fmod, args, (x, y) => x % y),
            TokenName.Builtin_Math_Hypot       => FloatBinary(token, MathBuiltin.Hypot, args, HypotImpl),
            TokenName.Builtin_Math_IsFinite    => FloatToBool(token, MathBuiltin.IsFinite, args, double.IsFinite),
            TokenName.Builtin_Math_IsInf       => FloatToBool(token, MathBuiltin.IsInf, args, double.IsInfinity),
            TokenName.Builtin_Math_IsNaN       => FloatToBool(token, MathBuiltin.IsNaN, args, double.IsNaN),
            TokenName.Builtin_Math_IsNormal    => FloatToBool(token, MathBuiltin.IsNormal, args, double.IsNormal),
            TokenName.Builtin_Math_LGamma      => FloatUnary(token, MathBuiltin.LGamma, args, LGammaApprox),
            TokenName.Builtin_Math_Log         => Log(token, args),
            TokenName.Builtin_Math_Log10       => FloatUnary(token, MathBuiltin.Log10, args, Math.Log10),
            TokenName.Builtin_Math_Log1P       => FloatUnary(token, MathBuiltin.Log1P, args, x => Math.Log(1.0 + x)),
            TokenName.Builtin_Math_Log2        => FloatUnary(token, MathBuiltin.Log2, args, Math.Log2),
            TokenName.Builtin_Math_NextAfter   => NextAfter(token, args),
            TokenName.Builtin_Math_Pow         => FloatBinary(token, MathBuiltin.Pow, args, Math.Pow),
            TokenName.Builtin_Math_Random      => Random(token, args),
            TokenName.Builtin_Math_RandomSet   => RandomSet(token, args),
            TokenName.Builtin_Math_Remainder   => FloatBinary(token, MathBuiltin.Remainder, args, Math.IEEERemainder),
            TokenName.Builtin_Math_RotateLeft  => RotateLeft(token, args),
            TokenName.Builtin_Math_RotateRight => RotateRight(token, args),
            TokenName.Builtin_Math_Round       => Round(token, args),
            TokenName.Builtin_Math_Sin         => FloatUnary(token, MathBuiltin.Sin, args, Math.Sin),
            TokenName.Builtin_Math_Sinh        => FloatUnary(token, MathBuiltin.Sinh, args, Math.Sinh),
            TokenName.Builtin_Math_Sqrt        => FloatUnary(token, MathBuiltin.Sqrt, args, Math.Sqrt),
            TokenName.Builtin_Math_Tan         => FloatUnary(token, MathBuiltin.Tan, args, Math.Tan),
            TokenName.Builtin_Math_Tanh        => FloatUnary(token, MathBuiltin.Tanh, args, Math.Tanh),
            TokenName.Builtin_Math_TGamma      => FloatUnary(token, MathBuiltin.TGamma, args, TGammaApprox),
            TokenName.Builtin_Math_Trunc       => FloatUnary(token, MathBuiltin.Trunc, args, Math.Truncate),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    // --- dispatch helpers ---

    private static Value FloatUnary(Token token, string name, List<Value> args, Func<double, double> fn)
    {
        ParameterCountMismatchError.Check(token, name, 1, args.Count);
        ParameterTypeMismatchError.ExpectFloat(token, name, 0, args[0]);
        return Value.CreateFloat(fn(args[0].GetFloat()));
    }

    private static Value FloatBinary(Token token, string name, List<Value> args, Func<double, double, double> fn)
    {
        ParameterCountMismatchError.Check(token, name, 2, args.Count);
        ParameterTypeMismatchError.ExpectFloat(token, name, 0, args[0]);
        ParameterTypeMismatchError.ExpectFloat(token, name, 1, args[1]);
        return Value.CreateFloat(fn(args[0].GetFloat(), args[1].GetFloat()));
    }

    private static Value FloatToBool(Token token, string name, List<Value> args, Func<double, bool> fn)
    {
        ParameterCountMismatchError.Check(token, name, 1, args.Count);
        ParameterTypeMismatchError.ExpectFloat(token, name, 0, args[0]);
        return Value.CreateBoolean(fn(args[0].GetFloat()));
    }

    // --- builtins ---

    private static Value Abs(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MathBuiltin.Abs, 1, args.Count);
        ParameterTypeMismatchError.ExpectNumber(token, MathBuiltin.Abs, 0, args[0]);

        if (args[0].IsInteger())
            return Value.CreateInteger(Math.Abs(args[0].GetInteger()));

        return Value.CreateFloat(Math.Abs(args[0].GetFloat()));
    }

    private static Value Divisors(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MathBuiltin.Divisors, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, MathBuiltin.Divisors, 0, args[0]);

        long n = Math.Abs(args[0].GetInteger());
        List<Value> result = [];

        for (long i = 1; i * i <= n; i++)
        {
            if (n % i == 0)
            {
                result.Add(Value.CreateInteger(i));
                if (i != n / i)
                    result.Add(Value.CreateInteger(n / i));
            }
        }

        result.Sort();
        return Value.CreateList(result);
    }

    private static Value Eps(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MathBuiltin.Epsilon, 0, args.Count);
        return Value.CreateFloat(double.Epsilon);
    }

    private static Value Log(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, MathBuiltin.Log, 1, 2, args.Count);
        ParameterTypeMismatchError.ExpectFloat(token, MathBuiltin.Log, 0, args[0]);

        if (args.Count == 2)
        {
            ParameterTypeMismatchError.ExpectFloat(token, MathBuiltin.Log, 1, args[1]);
            return Value.CreateFloat(Math.Log(args[0].GetFloat(), args[1].GetFloat()));
        }

        return Value.CreateFloat(Math.Log(args[0].GetFloat()));
    }

    private static Value NextAfter(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MathBuiltin.NextAfter, 2, args.Count);
        ParameterTypeMismatchError.ExpectFloat(token, MathBuiltin.NextAfter, 0, args[0]);
        ParameterTypeMismatchError.ExpectFloat(token, MathBuiltin.NextAfter, 1, args[1]);

        double x = args[0].GetFloat();
        double y = args[1].GetFloat();

        if (double.IsNaN(x) || double.IsNaN(y)) return Value.CreateFloat(double.NaN);
        if (x == y) return Value.CreateFloat(x);

        return Value.CreateFloat(x < y ? Math.BitIncrement(x) : Math.BitDecrement(x));
    }

    private static Value Random(Token token, List<Value> args)
    {
        if (args.Count != 0 && args.Count != 2)
            throw new ParameterCountMismatchError(token, MathBuiltin.Random);

        if (args.Count == 2)
        {
            if (args[0].IsInteger() && args[1].IsInteger())
            {
                List<long> ints = [args[0].GetInteger(), args[1].GetInteger()];
                return Value.CreateInteger(System.Random.Shared.NextInt64(ints.Min(), ints.Max() + 1));
            }
            else if ((args[0].IsFloat() || args[0].IsInteger()) && (args[1].IsFloat() || args[1].IsInteger())
                     && (args[0].IsFloat() || args[1].IsFloat()))
            {
                double minVal = args[0].IsFloat() ? args[0].GetFloat() : (double)args[0].GetInteger();
                double maxVal = args[1].IsFloat() ? args[1].GetFloat() : (double)args[1].GetInteger();
                if (minVal > maxVal) (minVal, maxVal) = (maxVal, minVal);
                return Value.CreateFloat(minVal + (maxVal - minVal) * System.Random.Shared.NextDouble());
            }
            else if (args[0].IsString() && args[1].IsInteger())
            {
                var allowedChars = args[0].GetString();
                var length = (int)args[1].GetInteger();
                var sb = new System.Text.StringBuilder(length);
                for (int i = 0; i < length; i++)
                    sb.Append(allowedChars[System.Random.Shared.Next(allowedChars.Length)]);
                return Value.CreateString(sb.ToString());
            }
            else if (args[0].IsList() && args[1].IsInteger())
            {
                var allowedValues = args[0].GetList();
                var length = (int)args[1].GetInteger();
                List<Value> list = [];
                for (int i = 0; i < length; i++)
                    list.Add(allowedValues[System.Random.Shared.Next(allowedValues.Count)]);
                return Value.CreateList(list);
            }
        }

        return Value.CreateFloat(System.Random.Shared.NextDouble());
    }

    private static Value RandomSet(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MathBuiltin.RandomSet, 3, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, MathBuiltin.RandomSet, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, MathBuiltin.RandomSet, 1, args[1]);
        ParameterTypeMismatchError.ExpectInteger(token, MathBuiltin.RandomSet, 2, args[2]);

        var min  = (int)args[0].GetInteger();
        var max  = (int)args[1].GetInteger();
        var size = (int)args[2].GetInteger();

        if (min > max) 
        {
            (min, max) = (max, min);
        }

        int rangeLen = max - min + 1;
        if (size <= 0 || rangeLen < size) 
        {
            return Value.CreateList([]);
        }

        var pool = new int[rangeLen];
        for (int i = 0; i < rangeLen; i++) 
        {
            pool[i] = min + i;
        }

        // Partial Fisher-Yates: shuffle only the first `size` positions
        for (int i = 0; i < size; i++)
        {
            int j = i + System.Random.Shared.Next(rangeLen - i);
            (pool[i], pool[j]) = (pool[j], pool[i]);
        }

        var result = new List<Value>(size);
        for (int i = 0; i < size; i++)
        {
            result.Add(Value.CreateInteger(pool[i]));
        }

        return Value.CreateList(result);
    }

    private static Value RotateLeft(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MathBuiltin.RotateLeft, 2, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, MathBuiltin.RotateLeft, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, MathBuiltin.RotateLeft, 1, args[1]);

        ulong value = (ulong)args[0].GetInteger();
        int count = (int)(args[1].GetInteger() & 63);
        return Value.CreateInteger((long)((value << count) | (value >> (64 - count))));
    }

    private static Value RotateRight(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MathBuiltin.RotateRight, 2, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, MathBuiltin.RotateRight, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, MathBuiltin.RotateRight, 1, args[1]);

        ulong value = (ulong)args[0].GetInteger();
        int count = (int)(args[1].GetInteger() & 63);
        return Value.CreateInteger((long)((value >> count) | (value << (64 - count))));
    }

    private static Value Round(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, MathBuiltin.Round, 1, 2, args.Count);
        ParameterTypeMismatchError.ExpectFloat(token, MathBuiltin.Round, 0, args[0]);

        if (args.Count == 2)
        {
            ParameterTypeMismatchError.ExpectInteger(token, MathBuiltin.Round, 1, args[1]);
            return Value.CreateFloat(Math.Round(args[0].GetFloat(), (int)args[1].GetInteger()));
        }

        return Value.CreateFloat(Math.Round(args[0].GetFloat()));
    }

    // --- numeric helpers ---

    // Numerically stable: avoids overflow for large values
    private static double HypotImpl(double x, double y)
    {
        x = Math.Abs(x);
        y = Math.Abs(y);
        if (x < y) (x, y) = (y, x);
        if (x == 0.0) return 0.0;
        double r = y / x;
        return x * Math.Sqrt(1.0 + r * r);
    }

    private static double FDimImpl(double x, double y)
    {
        if (double.IsNaN(x) || double.IsNaN(y)) return double.NaN;
        return Math.Max(x - y, 0.0);
    }

    private static double FMaxImpl(double x, double y)
    {
        if (double.IsNaN(x)) return y;
        if (double.IsNaN(y)) return x;
        return Math.Max(x, y);
    }

    private static double FMinImpl(double x, double y)
    {
        if (double.IsNaN(x)) return y;
        if (double.IsNaN(y)) return x;
        return Math.Min(x, y);
    }

    // Abramowitz & Stegun 7.1.26, max error 1.5e-7
    private static double ErfApprox(double x)
    {
        if (double.IsNaN(x)) return double.NaN;
        double t = 1.0 / (1.0 + 0.3275911 * Math.Abs(x));
        double poly = t * (0.254829592
                   + t * (-0.284496736
                   + t * (1.421413741
                   + t * (-1.453152027
                   + t * 1.061405429))));
        double result = 1.0 - poly * Math.Exp(-(x * x));
        return Math.Sign(x) * result;
    }

    // Lanczos approximation (g=7, 9 coefficients)
    private static double TGammaApprox(double z)
    {
        if (z <= 0 && z == Math.Truncate(z))
            return double.NaN;

        if (z < 0.5)
            return Math.PI / (Math.Sin(Math.PI * z) * TGammaApprox(1.0 - z));

        z -= 1;
        double[] p =
        [
             0.99999999999980993,
             676.5203681218851,
            -1259.1392167224028,
             771.32342877765313,
            -176.61502916214059,
              12.507343278686905,
              -0.13857109526572012,
               9.9843695780195716e-6,
               1.5056327351493116e-7
        ];

        double x = p[0];
        for (int i = 1; i < p.Length; i++)
            x += p[i] / (z + i);

        double t = z + 7.5;
        return Math.Sqrt(2.0 * Math.PI) * Math.Pow(t, z + 0.5) * Math.Exp(-t) * x;
    }

    private static double LGammaApprox(double x)
        => Math.Log(Math.Abs(TGammaApprox(x)));
}
