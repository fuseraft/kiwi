using System.Collections.Concurrent;
using System.Diagnostics;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Settings;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class SysBuiltinHandler
{
    // Tracks processes spawned via sys::spawn so sys::wait can retrieve exit codes.
    private static readonly ConcurrentDictionary<int, Process> _spawned = new();

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        if (Kiwi.Settings.SafeMode) throw new SafeModeError(token, "sys");
        return builtin switch
        {
            TokenName.Builtin_Sys_Exec    => Exec(token, args),
            TokenName.Builtin_Sys_ExecOut => ExecOut(token, args),
            TokenName.Builtin_Sys_Kill    => Kill(token, args),
            TokenName.Builtin_Sys_Open    => Open(token, args),
            TokenName.Builtin_Sys_Pid     => Pid(token, args),
            TokenName.Builtin_Sys_Pids      => Pids(token, args),
            TokenName.Builtin_Sys_Processes => Processes(token, args),
            TokenName.Builtin_Sys_Running   => Running(token, args),
            TokenName.Builtin_Sys_Spawn   => Spawn(token, args),
            TokenName.Builtin_Sys_Wait    => Wait(token, args),
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

    private static Value Pids(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Pids, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SysBuiltin.Pids, 0, args[0]);

        var name = args[0].GetString();
        Process[] processes = name.Length == 0
            ? Process.GetProcesses()
            : Process.GetProcessesByName(name);

        var list = processes
            .Select(p => { var id = p.Id; p.Dispose(); return Value.CreateInteger(id); })
            .ToList();

        return Value.CreateList(list);
    }

    private static Value Processes(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Processes, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SysBuiltin.Processes, 0, args[0]);

        var name = args[0].GetString();
        Process[] procs = name.Length == 0
            ? Process.GetProcesses()
            : Process.GetProcessesByName(name);

        var list = new List<Value>(procs.Length);

        foreach (var p in procs)
        {
            using (p)
            {
                string path    = "";
                long   memory  = 0;
                int    threads = 0;
                string started = "";

                try { path    = p.MainModule?.FileName ?? ""; } catch { }
                try { memory  = p.WorkingSet64;               } catch { }
                try { threads = p.Threads.Count;              } catch { }
                try { started = p.StartTime.ToString("o");    } catch { }

                var entry = new Dictionary<Value, Value>
                {
                    [Value.CreateString("pid")]     = Value.CreateInteger(p.Id),
                    [Value.CreateString("name")]    = Value.CreateString(p.ProcessName),
                    [Value.CreateString("path")]    = Value.CreateString(path),
                    [Value.CreateString("memory")]  = Value.CreateInteger(memory),
                    [Value.CreateString("threads")] = Value.CreateInteger(threads),
                    [Value.CreateString("started")] = Value.CreateString(started),
                };

                list.Add(Value.CreateHashmap(entry));
            }
        }

        return Value.CreateList(list);
    }

    private static Value Pid(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Pid, 0, args.Count);
        return Value.CreateInteger(Environment.ProcessId);
    }

    private static Value Spawn(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Spawn, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SysBuiltin.Spawn, 0, args[0]);
        ParameterTypeMismatchError.ExpectList(token, SysBuiltin.Spawn, 1, args[1]);

        var program   = args[0].GetString();
        var argValues = args[1].GetList();

        try
        {
            var psi = CreateStartInfo(token, program, argValues, redirect: false);
            var process = new Process { StartInfo = psi };
            process.Start();
            _spawned[process.Id] = process;
            return Value.CreateInteger(process.Id);
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"Failed to spawn '{program}': {ex.Message}");
        }
    }

    private static Value Kill(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Kill, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, SysBuiltin.Kill, 0, args[0]);

        var pid = (int)args[0].GetInteger();

        try
        {
            if (_spawned.TryRemove(pid, out var tracked))
            {
                if (!tracked.HasExited)
                {
                    tracked.Kill(entireProcessTree: true);
                    tracked.WaitForExit();
                }
                tracked.Dispose();
            }
            else
            {
                using var p = Process.GetProcessById(pid);
                p.Kill(entireProcessTree: true);
            }
            return Value.CreateBoolean(true);
        }
        catch
        {
            return Value.CreateBoolean(false);
        }
    }

    private static Value Running(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Running, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, SysBuiltin.Running, 0, args[0]);

        var pid = (int)args[0].GetInteger();

        try
        {
            if (_spawned.TryGetValue(pid, out var tracked))
                return Value.CreateBoolean(!tracked.HasExited);

            using var p = Process.GetProcessById(pid);
            return Value.CreateBoolean(!p.HasExited);
        }
        catch
        {
            return Value.CreateBoolean(false);
        }
    }

    private static Value Wait(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SysBuiltin.Wait, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, SysBuiltin.Wait, 0, args[0]);

        var pid = (int)args[0].GetInteger();

        try
        {
            if (_spawned.TryRemove(pid, out var tracked))
            {
                tracked.WaitForExit();
                var code = tracked.ExitCode;
                tracked.Dispose();
                return Value.CreateInteger(code);
            }

            using var p = Process.GetProcessById(pid);
            p.WaitForExit();
            return Value.CreateInteger(p.ExitCode);
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"Failed to wait for process {pid}: {ex.Message}");
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