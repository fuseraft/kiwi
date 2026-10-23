using System.Collections.Immutable;
using Kiwi.Native.Generator;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Xunit;

namespace Kiwi.Native.Tests;

public class GeneratorTests
{
    private static readonly ImmutableArray<MetadataReference> References =
    [
        .. ((string)AppContext.GetData("TRUSTED_PLATFORM_ASSEMBLIES")!)
            .Split(Path.PathSeparator)
            .Select(path => MetadataReference.CreateFromFile(path)),
        MetadataReference.CreateFromFile(typeof(KiwiFunctionAttribute).Assembly.Location),
    ];

    // Runs the generator over `source` and returns its diagnostics, the generated code, and
    // the errors of compiling the result.
    private static (ImmutableArray<Diagnostic> Generator, string Generated, ImmutableArray<Diagnostic> Compile) Run(string source, bool allowUnsafe = true)
    {
        var compilation = CSharpCompilation.Create(
            "Test",
            [CSharpSyntaxTree.ParseText(source, new CSharpParseOptions(LanguageVersion.CSharp11))],
            References,
            new CSharpCompilationOptions(OutputKind.DynamicallyLinkedLibrary, allowUnsafe: allowUnsafe, nullableContextOptions: NullableContextOptions.Enable));

        var driver = CSharpGeneratorDriver.Create(new KiwiFunctionGenerator())
            .RunGeneratorsAndUpdateCompilation(compilation, out var output, out var diagnostics);

        var generated = string.Concat(driver.GetRunResult().GeneratedTrees.Select(t => t.ToString()));
        var errors = output.GetDiagnostics().Where(d => d.Severity == DiagnosticSeverity.Error).ToImmutableArray();
        return (diagnostics, generated, errors);
    }

    private static string Ids(ImmutableArray<Diagnostic> diagnostics) =>
        string.Join(", ", diagnostics.Select(d => d.Id));

    [Fact]
    public void ExportsEveryFunctionAndCompiles()
    {
        var (diagnostics, generated, errors) = Run("""
            using System.Collections.Generic;
            using Kiwi.Native;

            public static class Ext
            {
                [KiwiFunction] public static long Add(long a, long b) => a + b;
                [KiwiFunction] public static double Scale(double x, double factor = 2.5) => x * factor;
                [KiwiFunction] public static float Tenth(float x = 1.5f) => x / 10;
                [KiwiFunction] public static string Greet(string? name = null, string greeting = "Hi \"you\"") => greeting;
                [KiwiFunction] public static long Sum(params long[] xs) => 0;
                [KiwiFunction] public static long? Maybe(long? x) => x;
                [KiwiFunction] public static Dictionary<string, List<long[]>> Nested(IReadOnlyDictionary<string, IEnumerable<long>?> m) => new();
                [KiwiFunction] public static IEnumerable<KeyValuePair<long, string?>> Pairs() => new Dictionary<long, string?>();
                [KiwiFunction] public static KiwiValue Raw(KiwiContext ctx, KiwiValue v, params KiwiValue[] rest) => v;
                [KiwiFunction] public static object? Echo(object? value) => value;
                [KiwiFunction] public static byte[]? Data(byte[] b, nint p, bool flag, int n) => b;
                [KiwiFunction] public static void Nothing() { }
                [KiwiFunction("custom")] internal static ulong Big() => 1;

                public static class Inner
                {
                    [KiwiFunction] public static decimal Money() => 1.5m;
                }
            }
            """);

        Assert.Empty(diagnostics);
        Assert.Empty(errors);
        Assert.Contains("EntryPoint = \"kiwi_ext_init\"", generated);
        Assert.Contains("Define(__ctx, \"add\", &F0, 2)", generated);
        Assert.Contains("Define(__ctx, \"scale\", &F1, -1)", generated);
        Assert.Contains("Define(__ctx, \"sum\", &F4, -1)", generated);
        Assert.Contains("Define(__ctx, \"raw\", &F8, -1)", generated);
        Assert.Contains("Define(__ctx, \"nothing\", &F11, 0)", generated);
        Assert.Contains("Define(__ctx, \"custom\", &F12, 0)", generated);
        Assert.Contains("Define(__ctx, \"money\", &F13, 0)", generated);
        Assert.Contains("CheckArgumentCount(\"scale\", __argc, 1, 2)", generated);
        Assert.Contains("CheckArgumentCount(\"raw\", __argc, 1, -1)", generated);
    }

    [Theory]
    [InlineData("[KiwiFunction] public static long F(System.DateTime d) => 0;", "KIWI001")]
    [InlineData("[KiwiFunction] public static long F(Dictionary<string?, long> d) => 0;", "KIWI001")]
    [InlineData("[KiwiFunction] public static System.Threading.Tasks.Task F() => null!;", "KIWI002")]
    [InlineData("[KiwiFunction] public long F() => 0;", "KIWI003")]
    [InlineData("[KiwiFunction] private static long F() => 0;", "KIWI004")]
    [InlineData("[KiwiFunction] public static long F<T>() => 0;", "KIWI005")]
    [InlineData("[KiwiFunction] public static long F(ref long x) => 0;", "KIWI006")]
    [InlineData("[KiwiFunction] public static long F(long x, KiwiContext ctx) => 0;", "KIWI007")]
    [InlineData("[KiwiFunction(\"a-b\")] public static long F() => 0;", "KIWI009")]
    public void ReportsUnsupportedMethods(string member, string id)
    {
        var (diagnostics, generated, _) = Run($$"""
            using System.Collections.Generic;
            using Kiwi.Native;
            public class Ext { {{member}} }
            """);

        Assert.Equal(id, Ids(diagnostics));
        Assert.Equal("", generated);
    }

    [Fact]
    public void ReportsAPrivateContainingType()
    {
        var (diagnostics, _, _) = Run("""
            using Kiwi.Native;
            public static class Ext { private static class Hidden { [KiwiFunction] public static long F() => 0; } }
            """);
        Assert.Equal("KIWI004", Ids(diagnostics));
    }

    [Fact]
    public void ReportsAGenericContainingType()
    {
        var (diagnostics, _, _) = Run("""
            using Kiwi.Native;
            public static class Ext<T> { [KiwiFunction] public static long F() => 0; }
            """);
        Assert.Equal("KIWI005", Ids(diagnostics));
    }

    [Fact]
    public void ReportsDuplicateNames()
    {
        var (diagnostics, _, _) = Run("""
            using Kiwi.Native;
            public static class Ext
            {
                [KiwiFunction] public static long SumAll() => 0;
                [KiwiFunction("sum_all")] public static long Other() => 0;
            }
            """);
        Assert.Equal("KIWI008", Ids(diagnostics));
    }

    [Fact]
    public void RequiresUnsafeCode()
    {
        var (diagnostics, _, _) = Run("""
            using Kiwi.Native;
            public static class Ext { [KiwiFunction] public static long F() => 0; }
            """, allowUnsafe: false);
        Assert.Equal("KIWI010", Ids(diagnostics));
    }

    [Fact]
    public void GeneratesNothingWithoutFunctions()
    {
        var (diagnostics, generated, errors) = Run("public static class Ext { }", allowUnsafe: false);
        Assert.Empty(diagnostics);
        Assert.Equal("", generated);
        Assert.Empty(errors);
    }

    [Theory]
    [InlineData("Add", "add")]
    [InlineData("SumSquares", "sum_squares")]
    [InlineData("ParseHTTPHeader", "parse_http_header")]
    [InlineData("ToUtf8", "to_utf8")]
    [InlineData("Base64Encode", "base64_encode")]
    [InlineData("already_snake", "already_snake")]
    [InlineData("IO", "io")]
    public void NamesFunctionsInSnakeCase(string method, string kiwi)
    {
        Assert.Equal(kiwi, KiwiFunctionGenerator.SnakeCase(method));
    }
}
