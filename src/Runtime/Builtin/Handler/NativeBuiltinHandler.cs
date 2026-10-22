using System.Runtime.InteropServices;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Native;
using kiwi.Settings;
using kiwi.Tracing.Error;
using kiwi.Typing;
using kiwi.VM;

namespace kiwi.Runtime.Builtin.Handler;

public static class NativeBuiltinHandler
{
    public static Value Execute(KiwiVM vm, Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Native_Load => Load(vm, token, args),
            TokenName.Builtin_Native_Rid  => Rid(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Load(KiwiVM vm, Token token, List<Value> args)
    {
        if (Kiwi.Settings.SafeMode) throw new SafeModeError(token, "native");
        ParameterCountMismatchError.CheckRange(token, NativeBuiltin.Load, 1, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, NativeBuiltin.Load, 0, args[0]);

        var path = ResolvePath(vm, args[0].GetString());
        var packageName = string.Empty;
        if (args.Count > 1 && !args[1].IsNull())
        {
            ParameterTypeMismatchError.ExpectString(token, NativeBuiltin.Load, 1, args[1]);
            packageName = args[1].GetString();
        }
        if (packageName.Length == 0)
            packageName = PackageNameFromPath(path);

        var ext = NativeHost.Load(vm, token, path, packageName);
        return Value.CreatePackage(ext.PackageName);
    }

    /// <summary>The portable runtime identifier, such as <c>linux-x64</c> or <c>osx-arm64</c>.</summary>
    private static Value Rid(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, NativeBuiltin.Rid, 0, args.Count);

        var os = OperatingSystem.IsWindows() ? "win"
               : OperatingSystem.IsMacOS()   ? "osx"
               : OperatingSystem.IsLinux()   ? "linux"
               : OperatingSystem.IsFreeBSD() ? "freebsd"
               : "unknown";
        var arch = RuntimeInformation.ProcessArchitecture.ToString().ToLowerInvariant();
        return Value.CreateString(os + "-" + arch);
    }

    /// <summary>
    /// A relative path that exists next to the running script is resolved against it;
    /// anything else is passed through for the platform's library search.
    /// </summary>
    private static string ResolvePath(KiwiVM vm, string path)
    {
        if (Path.IsPathRooted(path))
            return path;

        var scriptDir = Path.GetDirectoryName(vm.ExecutionPath);
        if (!string.IsNullOrEmpty(scriptDir))
        {
            var candidate = Path.GetFullPath(Path.Combine(scriptDir, path));
            if (File.Exists(candidate))
                return candidate;
        }

        return File.Exists(path) ? Path.GetFullPath(path) : path;
    }

    /// <summary><c>/x/libfoo-bar.so</c> → <c>foo_bar</c>.</summary>
    private static string PackageNameFromPath(string path)
    {
        var name = Path.GetFileName(path);
        var dot  = name.IndexOf('.');
        if (dot > 0) name = name[..dot];
        if (name.StartsWith("lib") && name.Length > 3) name = name[3..];

        var chars = name.Select(ch => char.IsLetterOrDigit(ch) || ch == '_' ? ch : '_').ToArray();
        if (chars.Length > 0 && char.IsDigit(chars[0]))
            return "_" + new string(chars);
        return new string(chars);
    }
}
