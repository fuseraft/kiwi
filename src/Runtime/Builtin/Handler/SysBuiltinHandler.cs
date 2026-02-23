using System.Diagnostics;
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
            TokenName.Builtin_Sys_Exec    => Exec(token, args),
            TokenName.Builtin_Sys_ExecOut => ExecOut(token, args),
            TokenName.Builtin_Sys_Open    => Open(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }
    
    private static Value Exec(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Exec, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SysBuiltin.Exec, 0, args[0]);
        ParameterTypeMismatchError.ExpectList(token, SysBuiltin.Exec, 1, args[1]);

        var program = args[0].GetString();
        var argValues = args[1].GetList();

        try
        {
            var psi = CreateStartInfo(token, program, argValues, redirect: false);
            psi.CreateNoWindow = true;

            using var process = new Process { StartInfo = psi };
            process.Start();
            process.WaitForExit();

            return Value.CreateInteger(process.ExitCode);
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"Failed to execute '{program}': {ex.Message}");
        }
    }

    private static Value ExecOut(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.ExecOut, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SysBuiltin.ExecOut, 0, args[0]);
        ParameterTypeMismatchError.ExpectList(token, SysBuiltin.ExecOut, 1, args[1]);

        var program = args[0].GetString();
        var argValues = args[1].GetList();

        try
        {
            var psi = CreateStartInfo(token, program, argValues, redirect: true);
            psi.CreateNoWindow = true;

            using var process = new Process { StartInfo = psi };
            process.Start();

            string stdout = process.StandardOutput.ReadToEnd();
            string stderr = process.StandardError.ReadToEnd();

            process.WaitForExit();

            var result = new Dictionary<Value, Value>
            {
                [Value.CreateString("stdout")]    = Value.CreateString(stdout),
                [Value.CreateString("stderr")]    = Value.CreateString(stderr),
                [Value.CreateString("exit_code")] = Value.CreateInteger(process.ExitCode)
            };

            return Value.CreateHashmap(result);
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"Failed to execute '{program}': {ex.Message}");
        }
    }

    private static Value Open(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Open, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SysBuiltin.Open, 0, args[0]);
        ParameterTypeMismatchError.ExpectList(token, SysBuiltin.Open, 1, args[1]);

        var program = args[0].GetString();
        var argValues = args[1].GetList();

        try
        {
            var psi = CreateStartInfo(token, program, argValues, redirect: false);
            using var process = new Process { StartInfo = psi };
            process.Start();
            return Value.CreateBoolean(true);
        }
        catch
        {
            return Value.CreateBoolean(false);
        }
    }

    private static ProcessStartInfo CreateStartInfo(Token token, string program, List<Value> argValues, bool redirect)
    {
        var psi = new ProcessStartInfo
        {
            FileName               = program,
            UseShellExecute        = false,
            RedirectStandardOutput = redirect,
            RedirectStandardError  = redirect,
            RedirectStandardInput  = false,
        };

        foreach (var arg in argValues)
        {
            TypeError.ExpectString(token, arg);
            psi.ArgumentList.Add(arg.GetString());
        }

        return psi;
    }
}