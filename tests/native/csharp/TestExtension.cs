using System.Text;
using Kiwi.Native;

namespace KiwiTestExtCs;

/// <summary>
/// Functions for tests/native/csharp_ext.kiwi. Between them they use every parameter and
/// return conversion the generator writes.
/// </summary>
public static class TestExtension
{
    // -- Scalars

    [KiwiFunction]
    public static long Add(long a, long b) => a + b;

    [KiwiFunction]
    public static double Scale(double x, double factor = 2.0) => x * factor;

    [KiwiFunction]
    public static int Half(int x) => x / 2;

    [KiwiFunction]
    public static bool Negate(bool b) => !b;

    [KiwiFunction]
    public static float Tenth(float x) => x / 10f;

    [KiwiFunction]
    public static string Reverse(string s)
    {
        var chars = s.ToCharArray();
        Array.Reverse(chars);
        return new string(chars);
    }

    [KiwiFunction]
    public static string Greet(string? name = null, string greeting = "Hello") => $"{greeting}, {name ?? "stranger"}!";

    [KiwiFunction]
    public static void Nothing() { }

    [KiwiFunction]
    public static long? Maybe(long? x) => x is null ? null : x * 10;

    [KiwiFunction("explicit_name")]
    public static string Renamed() => "renamed";

    [KiwiFunction]
    public static string ParseHTTPHeader() => "snake";

    // -- Variadic

    [KiwiFunction]
    public static long Sum(params long[] xs) => xs.Sum();

    [KiwiFunction]
    public static string Join(string separator, params string[] parts) => string.Join(separator, parts);

    // -- Collections

    [KiwiFunction]
    public static long[] Squares(long[] xs) => xs.Select(x => x * x).ToArray();

    [KiwiFunction]
    public static List<string> Upper(List<string> words) => words.Select(w => w.ToUpperInvariant()).ToList();

    [KiwiFunction]
    public static IEnumerable<long> Range(int n) => Enumerable.Range(0, n).Select(i => (long)i);

    [KiwiFunction]
    public static Dictionary<string, int> WordLengths(IEnumerable<string> words) =>
        words.Distinct().ToDictionary(w => w, w => w.Length);

    [KiwiFunction]
    public static Dictionary<string, object?> Describe(Dictionary<string, object?> map) => new()
    {
        ["has_name"] = map.ContainsKey("name"),
        ["keys"] = map.Keys.ToList(),
    };

    [KiwiFunction]
    public static long[][] Grid(int rows, int cols) =>
        Enumerable.Range(0, rows).Select(r => Enumerable.Range(0, cols).Select(c => (long)(r * cols + c)).ToArray()).ToArray();

    [KiwiFunction]
    public static long GridSum(List<long[]> grid) => grid.Sum(row => row.Sum());

    [KiwiFunction]
    public static List<string?> Compact(List<string?> items) => items.Where(s => s != null).ToList();

    [KiwiFunction]
    public static object? Echo(object? value) => value;

    // -- Bytes and pointers

    [KiwiFunction]
    public static byte[] BytesXor(byte[] data, int key) => data.Select(b => (byte)(b ^ key)).ToArray();

    [KiwiFunction]
    public static byte[] Utf8(string s) => Encoding.UTF8.GetBytes(s);

    private sealed class Counter
    {
        public long Value;
    }

    [KiwiFunction]
    public static nint CounterNew(long start = 0) => KiwiHandle.Alloc(new Counter { Value = start });

    [KiwiFunction]
    public static long CounterBump(nint counter) => ++KiwiHandle.Get<Counter>(counter).Value;

    [KiwiFunction]
    public static void CounterFree(nint counter) => KiwiHandle.Free(counter);

    // -- Raw handles and callbacks

    [KiwiFunction]
    public static KiwiValue Apply(KiwiContext ctx, KiwiValue fn, params KiwiValue[] args) => ctx.Call(fn, args);

    [KiwiFunction]
    public static List<object?> MapEach(KiwiContext ctx, KiwiValue fn, List<object?> items) =>
        items.Select(item => ctx.Invoke(fn, item)).ToList();

    [KiwiFunction]
    public static string TypeName(KiwiContext ctx, KiwiValue value) => ctx.TypeOf(value).ToString();

    [KiwiFunction]
    public static KiwiValue Pairs(KiwiContext ctx, KiwiValue map)
    {
        var list = ctx.NewList();
        foreach (var key in ctx.HashmapKeys(map))
        {
            ctx.TryGetValue(map, key, out var value);
            ctx.Push(list, ctx.NewList([key, value]));
        }
        return list;
    }

    [KiwiFunction]
    public static string CallbackSurvives(KiwiContext ctx, KiwiValue fn)
    {
        try
        {
            ctx.Call(fn);
            return "no error";
        }
        catch (KiwiException)
        {
            return "caught";
        }
    }

    // -- Errors

    [KiwiFunction]
    public static void Fail(string message) => throw new InvalidOperationException(message);

    [KiwiFunction]
    public static long Divide(long a, long b) => b == 0 ? throw new KiwiException("division by zero") : a / b;
}
