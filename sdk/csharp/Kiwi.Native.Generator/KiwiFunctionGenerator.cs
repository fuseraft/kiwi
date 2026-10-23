using System.Collections.Generic;
using System.Collections.Immutable;
using System.Text;
using System.Text.RegularExpressions;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;
using Microsoft.CodeAnalysis.CSharp.Syntax;

namespace Kiwi.Native.Generator;

/// <summary>
/// Exports every <c>[KiwiFunction]</c> method to Kiwi: writes <c>kiwi_ext_init</c>, which
/// registers them, and an <c>[UnmanagedCallersOnly]</c> wrapper for each that converts its
/// arguments and result and reports exceptions as Kiwi errors.
/// </summary>
[Generator(LanguageNames.CSharp)]
public sealed class KiwiFunctionGenerator : IIncrementalGenerator
{
    private const string AttributeName = "Kiwi.Native.KiwiFunctionAttribute";
    private static readonly Regex ValidName = new("^[A-Za-z_][A-Za-z0-9_]*$");

    public void Initialize(IncrementalGeneratorInitializationContext context)
    {
        var functions = context.SyntaxProvider.ForAttributeWithMetadataName(
            AttributeName,
            static (node, _) => node is MethodDeclarationSyntax,
            static (ctx, _) => Analyze((IMethodSymbol)ctx.TargetSymbol, ctx.Attributes[0], ctx.TargetNode.GetLocation()));

        var allowUnsafe = context.CompilationProvider.Select(static (c, _) => c.Options is CSharpCompilationOptions { AllowUnsafe: true });

        context.RegisterSourceOutput(functions.Collect().Combine(allowUnsafe), static (spc, input) => Emit(spc, input.Left, input.Right));
    }

    private sealed class Function
    {
        public string KiwiName = "";
        public string Display = "";
        public Location? Location;
        public int Arity;
        public string Body = "";
        public ImmutableArray<Diagnostic> Errors = ImmutableArray<Diagnostic>.Empty;
    }

    private static Function Analyze(IMethodSymbol method, AttributeData attribute, Location location)
    {
        var display = $"{method.ContainingType.Name}.{method.Name}";
        var explicitName = attribute.ConstructorArguments.Length > 0 ? attribute.ConstructorArguments[0].Value as string : null;
        var function = new Function
        {
            KiwiName = explicitName ?? SnakeCase(method.Name),
            Display  = display,
            Location = location,
        };

        var errors = ImmutableArray.CreateBuilder<Diagnostic>();
        void Report(DiagnosticDescriptor d, params object[] args) => errors.Add(Diagnostic.Create(d, location, args));

        if (!ValidName.IsMatch(function.KiwiName)) Report(Diagnostics.InvalidName, function.KiwiName);
        if (!method.IsStatic) Report(Diagnostics.NotStatic, display);
        if (!IsAccessible(method)) Report(Diagnostics.NotAccessible, display);
        if (method.IsGenericMethod || InGenericType(method)) Report(Diagnostics.Generic, display);

        // Parameters after an optional KiwiContext are Kiwi arguments.
        var parameters = method.Parameters;
        var first = parameters.Length > 0 && Marshalling.IsKiwiContext(parameters[0].Type) ? 1 : 0;

        var body = new StringBuilder();
        var callArgs = new List<string>();
        if (first == 1) callArgs.Add("__ctx");

        int required = 0, max = 0;
        var variadic = false;

        for (var i = first; i < parameters.Length; i++)
        {
            var p = parameters[i];
            var index = i - first;
            var local = "__p" + index;
            callArgs.Add(local);

            if (p.RefKind != RefKind.None) { Report(Diagnostics.ByRef, p.Name, display); continue; }
            if (Marshalling.IsKiwiContext(p.Type)) { Report(Diagnostics.ContextNotFirst, p.Name, display); continue; }

            var name = SymbolDisplay.FormatLiteral(p.Name, quote: true);

            if (p.IsParams && p.Type is IArrayTypeSymbol array)
            {
                variadic = true;
                var read = Marshalling.Read(array.ElementType, "__ctx", $"new {Marshalling.Value}(__argv[{index} + __i])", 0);
                if (read == null) { Report(Diagnostics.UnsupportedParameter, p.Name, display, Marshalling.Display(array.ElementType)); continue; }
                body.AppendLine($"                __name = {name};");
                body.AppendLine($"                var {local} = new {Marshalling.Display(array.ElementType)}[global::System.Math.Max(0, __argc - {index})];");
                body.AppendLine($"                for (var __i = 0; __i < {local}.Length; __i++) {{ __arg = {index} + __i; {local}[__i] = {read}; }}");
                continue;
            }

            var reader = Marshalling.Read(p.Type, "__ctx", $"new {Marshalling.Value}(__argv[{index}])", 0);
            if (reader == null) { Report(Diagnostics.UnsupportedParameter, p.Name, display, Marshalling.Display(p.Type)); continue; }

            max++;
            body.AppendLine($"                __arg = {index}; __name = {name};");
            if (p.HasExplicitDefaultValue)
            {
                var literal = Marshalling.DefaultLiteral(p);
                if (literal == null) { Report(Diagnostics.UnsupportedDefault, p.Name, display); continue; }
                body.AppendLine($"                var {local} = __argc > {index} ? {reader} : {literal};");
            }
            else
            {
                required++;
                body.AppendLine($"                var {local} = {reader};");
            }
        }

        string? write = "";
        var call = $"{Marshalling.Display(method.ContainingType)}.{method.Name}({string.Join(", ", callArgs)})";
        if (method.ReturnsVoid)
        {
            body.AppendLine("                __arg = -1;");
            body.AppendLine($"                {call};");
            body.AppendLine("                return 0;");
        }
        else
        {
            write = Marshalling.Write(method.ReturnType, "__ctx", "__result", 0);
            if (write == null)
            {
                Report(Diagnostics.UnsupportedReturn, display, Marshalling.Display(method.ReturnType));
            }
            else
            {
                body.AppendLine("                __arg = -1;");
                body.AppendLine($"                var __result = {call};");
                body.AppendLine($"                return ({write}).Handle;");
            }
        }

        // A fixed arity lets Kiwi check the count before the call; otherwise the wrapper does.
        var fixedArity = !variadic && required == max;
        function.Arity = fixedArity ? max : -1;
        var check = fixedArity ? "" :
            $"                global::Kiwi.Native.KiwiRuntime.CheckArgumentCount({SymbolDisplay.FormatLiteral(function.KiwiName, true)}, __argc, {required}, {(variadic ? -1 : max)});\n";

        function.Body = check + body;
        function.Errors = errors.ToImmutable();
        return function;
    }

    private static void Emit(SourceProductionContext spc, ImmutableArray<Function> functions, bool allowUnsafe)
    {
        if (functions.IsEmpty) return;

        var failed = false;
        foreach (var f in functions)
        {
            foreach (var e in f.Errors) spc.ReportDiagnostic(e);
            failed |= !f.Errors.IsEmpty;
        }

        var seen = new Dictionary<string, Function>();
        foreach (var f in functions)
        {
            if (seen.TryGetValue(f.KiwiName, out var other))
            {
                spc.ReportDiagnostic(Diagnostic.Create(Diagnostics.DuplicateName, f.Location, other.Display, f.Display, f.KiwiName));
                failed = true;
            }
            else
            {
                seen[f.KiwiName] = f;
            }
        }

        if (!allowUnsafe)
        {
            spc.ReportDiagnostic(Diagnostic.Create(Diagnostics.UnsafeRequired, functions[0].Location));
            failed = true;
        }

        if (failed) return;

        const string cdecl = "CallConvs = new[] { typeof(global::System.Runtime.CompilerServices.CallConvCdecl) }";
        var sb = new StringBuilder();
        sb.AppendLine("// <auto-generated/>");
        sb.AppendLine("#nullable enable");
        sb.AppendLine("#pragma warning disable");
        sb.AppendLine();
        sb.AppendLine("namespace Kiwi.Native.Generated");
        sb.AppendLine("{");
        sb.AppendLine("    [global::System.CodeDom.Compiler.GeneratedCode(\"Kiwi.Native.Generator\", \"0.1.0\")]");
        sb.AppendLine("    internal static unsafe class KiwiExtension");
        sb.AppendLine("    {");
        sb.AppendLine($"        [global::System.Runtime.InteropServices.UnmanagedCallersOnly(EntryPoint = \"kiwi_ext_init\", {cdecl})]");
        sb.AppendLine("        private static int Init(void* __api, nint __ctx)");
        sb.AppendLine("        {");
        sb.AppendLine("            try");
        sb.AppendLine("            {");
        sb.AppendLine("                if (!global::Kiwi.Native.KiwiRuntime.Initialize(__api)) return 1;");
        sb.AppendLine("                var __ok = true;");
        for (var i = 0; i < functions.Length; i++)
            sb.AppendLine($"                __ok &= global::Kiwi.Native.KiwiRuntime.Define(__ctx, {SymbolDisplay.FormatLiteral(functions[i].KiwiName, true)}, &F{i}, {functions[i].Arity});");
        sb.AppendLine("                return __ok ? 0 : 1;");
        sb.AppendLine("            }");
        sb.AppendLine("            catch (global::System.Exception __e)");
        sb.AppendLine("            {");
        sb.AppendLine("                return global::Kiwi.Native.KiwiRuntime.FailInit(__ctx, __e);");
        sb.AppendLine("            }");
        sb.AppendLine("        }");

        for (var i = 0; i < functions.Length; i++)
        {
            var f = functions[i];
            sb.AppendLine();
            sb.AppendLine($"        // {f.KiwiName}: {f.Display}");
            sb.AppendLine($"        [global::System.Runtime.InteropServices.UnmanagedCallersOnly({cdecl})]");
            sb.AppendLine($"        private static ulong F{i}(nint __handle, int __argc, ulong* __argv)");
            sb.AppendLine("        {");
            sb.AppendLine("            var __ctx = global::Kiwi.Native.KiwiRuntime.Enter(__handle);");
            sb.AppendLine("            var __arg = -1;");
            sb.AppendLine("            string? __name = null;");
            sb.AppendLine("            try");
            sb.AppendLine("            {");
            sb.Append(f.Body);
            sb.AppendLine("            }");
            sb.AppendLine("            catch (global::System.Exception __e)");
            sb.AppendLine("            {");
            sb.AppendLine($"                return global::Kiwi.Native.KiwiRuntime.Fail(__handle, {SymbolDisplay.FormatLiteral(f.KiwiName, true)}, __arg, __name, __e);");
            sb.AppendLine("            }");
            sb.AppendLine("        }");
        }

        sb.AppendLine("    }");
        sb.AppendLine("}");
        spc.AddSource("KiwiExtension.g.cs", sb.ToString());
    }

    private static bool IsAccessible(IMethodSymbol method)
    {
        static bool Ok(Accessibility a) => a is Accessibility.Public or Accessibility.Internal or Accessibility.ProtectedOrInternal;
        if (!Ok(method.DeclaredAccessibility)) return false;
        for (var t = method.ContainingType; t != null; t = t.ContainingType)
            if (!Ok(t.DeclaredAccessibility)) return false;
        return true;
    }

    private static bool InGenericType(IMethodSymbol method)
    {
        for (var t = method.ContainingType; t != null; t = t.ContainingType)
            if (t.IsGenericType) return true;
        return false;
    }

    /// <summary><c>SumSquares</c> → <c>sum_squares</c>, <c>ParseHTTPHeader</c> → <c>parse_http_header</c>.</summary>
    internal static string SnakeCase(string name)
    {
        var sb = new StringBuilder(name.Length + 4);
        for (var i = 0; i < name.Length; i++)
        {
            var c = name[i];
            if (char.IsUpper(c))
            {
                var prev = i > 0 ? name[i - 1] : '\0';
                var next = i + 1 < name.Length ? name[i + 1] : '\0';
                var boundary = i > 0 && prev != '_' &&
                    (char.IsLower(prev) || char.IsDigit(prev) || (char.IsUpper(prev) && char.IsLower(next)));
                if (boundary) sb.Append('_');
                sb.Append(char.ToLowerInvariant(c));
            }
            else
            {
                sb.Append(c);
            }
        }
        return sb.ToString();
    }
}
