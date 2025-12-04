using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class SysBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Sys_Exec => Exec(token, args),
            TokenName.Builtin_Sys_ExecOut => ExecOut(token, args),
            TokenName.Builtin_Sys_Open => Open(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Open(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Open, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, SysBuiltin.Open, 0, args[0]);

        var process = new System.Diagnostics.Process
        {
            StartInfo = new System.Diagnostics.ProcessStartInfo
            {
                FileName = args[0].GetString()
            }
        };

        process.Start();

        return Value.Default;
    }

    private static Value Exec(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Exec, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, SysBuiltin.Exec, 0, args[0]);

        var startInfo = GetStartInfo(args[0].GetString());
        startInfo.CreateNoWindow = true;

        var process = new System.Diagnostics.Process
        {
            StartInfo = startInfo
        };

        process.Start();
        process.WaitForExit();

        return Value.CreateInteger(process.ExitCode);
    }

    private static Value ExecOut(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.ExecOut, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, SysBuiltin.ExecOut, 0, args[0]);

        var startInfo = GetStartInfo(args[0].GetString());
        startInfo.RedirectStandardOutput = true;
        startInfo.RedirectStandardError = true;
        startInfo.CreateNoWindow = true;

        var process = new System.Diagnostics.Process
        {
            StartInfo = startInfo
        };

        process.Start();

        var output = process.StandardOutput.ReadToEnd();
        var error = process.StandardError.ReadToEnd();

        process.WaitForExit();

        Dictionary<Value, Value> execOut = [];
        execOut[Value.CreateString("stdout")] = Value.CreateString(output);
        execOut[Value.CreateString("stderr")] = Value.CreateString(error);

        return Value.CreateHashmap(execOut);
    }

    private static System.Diagnostics.ProcessStartInfo GetStartInfo(string command)
    {
        return new System.Diagnostics.ProcessStartInfo
        {
            FileName = OperatingSystem.IsWindows() ? "cmd.exe" : "bash",
            Arguments = (OperatingSystem.IsWindows() ? "/c " : "-c ") + command
        };
    }
}
