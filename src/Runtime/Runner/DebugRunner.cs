using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Runtime.Builtin.Util;
using kiwi.Runtime.Debugger;
using kiwi.Settings;
using kiwi.Tracing;
using kiwi.Tracing.Error;
using kiwi.Typing;
using kiwi.VM;

namespace kiwi.Runtime.Runner;

public class DebugRunner(Interpreter interpreter) : VMScriptRunner(interpreter)
{
    private readonly DebugState _state = new();
    private readonly HashSet<string> _stdlibDirs = [];
    private KiwiVM? _currentVM;
    private string _currentFile = string.Empty;
    private int _currentLine = -1;

    public override int Run(string script, List<string> args)
    {
        ComputeStdlibDirs();

        Console.WriteLine("kdb - Kiwi Debugger");
        Console.WriteLine($"Debugging: {script}");
        Console.WriteLine("Type 'h' for help, 's' to step, 'r' to run.");
        Console.WriteLine();

        _state.Mode = DebugMode.StepIn;

        return base.Run(script, args);
    }

    protected override void ConfigureVM(KiwiVM vm)
    {
        _currentVM = vm;
        vm.DebugHook = OnVMDebugHook;
    }

    private void ComputeStdlibDirs()
    {
        var exeDir = Path.GetDirectoryName(Environment.ProcessPath ?? string.Empty) ?? string.Empty;

        foreach (var library in Kiwi.Settings.StandardLibrary)
        {
            var libPath = library.IsOverride
                ? FileUtil.ExpandPath(library.Path)
                : Path.GetFullPath(Path.Combine(exeDir, library.Path));

            _stdlibDirs.Add(libPath);
        }
    }

    private bool IsStdlibFile(string file)
    {
        foreach (var dir in _stdlibDirs)
        {
            if (file.StartsWith(dir + Path.DirectorySeparatorChar) ||
                file.StartsWith(dir + '/'))
            {
                return true;
            }
        }

        return false;
    }

    private void OnVMDebugHook(int fileId, int line)
    {
        var file = FileRegistry.Instance.GetFilePath(fileId);

        if (string.IsNullOrEmpty(file) || IsStdlibFile(file))
        {
            return;
        }

        if (file == _state.LastFile && line == _state.LastLine)
        {
            return;
        }

        var depth = _currentVM!.FrameCount;

        var shouldPause = _state.Mode switch
        {
            DebugMode.StepIn   => true,
            DebugMode.StepOver => depth <= _state.StepDepth,
            DebugMode.StepOut  => depth < _state.StepDepth,
            DebugMode.Running  => _state.HasBreakpoint(file, line),
            _                  => false,
        };

        if (!shouldPause)
        {
            return;
        }

        _state.LastFile = file;
        _state.LastLine = line;
        _currentFile = file;
        _currentLine = line;

        ShowContext(file, line);
        RunDebugREPL();
    }

    // -- Color helpers ---------------------------------------------------------

    private static void C(ConsoleColor color, string text, bool newline = false)
    {
        Console.ForegroundColor = color;
        if (newline) Console.WriteLine(text); else Console.Write(text);
        Console.ResetColor();
    }

    private static void Err(string msg) => C(ConsoleColor.Red, msg, newline: true);
    private static void Ok(string msg)  => C(ConsoleColor.Green, msg, newline: true);

    // -------------------------------------------------------------------------

    private void RunDebugREPL()
    {
        while (true)
        {
            C(ConsoleColor.Yellow, "(kdb) ");
            var input = Console.ReadLine()?.Trim() ?? string.Empty;

            if (string.IsNullOrEmpty(input))
            {
                continue;
            }

            var parts = input.Split(' ', 2, StringSplitOptions.RemoveEmptyEntries);
            var cmd = parts[0].ToLower();
            var arg = parts.Length > 1 ? parts[1].Trim() : string.Empty;

            switch (cmd)
            {
                case "r":
                case "run":
                    _state.Mode = DebugMode.Running;
                    return;

                case "s":
                case "step":
                    _state.Mode = DebugMode.StepIn;
                    _state.StepDepth = _currentVM?.FrameCount ?? 0;
                    return;

                case "n":
                case "next":
                    _state.Mode = DebugMode.StepOver;
                    _state.StepDepth = _currentVM?.FrameCount ?? 0;
                    return;

                case "f":
                case "finish":
                    _state.Mode = DebugMode.StepOut;
                    _state.StepDepth = _currentVM?.FrameCount ?? 0;
                    return;

                case "b":
                    HandleBreakpoint(arg);
                    break;

                case "d":
                    HandleDeleteBreakpoint(arg);
                    break;

                case "info":
                    if (arg == "b") ListBreakpoints();
                    break;

                case "p":
                    EvalExpression(arg);
                    break;

                case "l":
                case "locals":
                    ShowLocals();
                    break;

                case "bt":
                case "backtrace":
                    ShowBacktrace();
                    break;

                case "list":
                    ShowSource(arg);
                    break;

                case "h":
                case "help":
                    ShowHelp();
                    break;

                case "q":
                case "quit":
                    Environment.Exit(0);
                    break;

                default:
                    Err($"Unknown command: '{cmd}'. Type 'h' for help.");
                    break;
            }
        }
    }

    private void HandleBreakpoint(string arg)
    {
        if (string.IsNullOrWhiteSpace(arg))
        {
            Console.WriteLine("Usage: b <line>  or  b <file>:<line>");
            return;
        }

        string file;
        int line;

        // Check for file:line format (handle Windows paths with drive letters by using LastIndexOf)
        var colonIdx = arg.LastIndexOf(':');
        if (colonIdx > 1 && int.TryParse(arg[(colonIdx + 1)..], out line))
        {
            file = arg[..colonIdx];
        }
        else if (int.TryParse(arg, out line))
        {
            file = _state.LastFile;
        }
        else
        {
            Console.WriteLine($"Invalid breakpoint: '{arg}'. Use 'b <line>' or 'b <file>:<line>'.");
            return;
        }

        _state.AddBreakpoint(file, line);
        var location = string.IsNullOrEmpty(file) ? $"{line}" : $"{file}:{line}";
        Ok($"Breakpoint {_state.Breakpoints.Count} at {location}");
    }

    private void HandleDeleteBreakpoint(string arg)
    {
        if (!int.TryParse(arg, out var index))
        {
            Err("Usage: d <n>  (where n is the breakpoint number from 'info b')");
            return;
        }

        if (_state.RemoveBreakpoint(index))
        {
            Ok($"Deleted breakpoint {index}");
        }
        else
        {
            Err($"No breakpoint #{index}");
        }
    }

    private void ListBreakpoints()
    {
        if (_state.Breakpoints.Count == 0)
        {
            C(ConsoleColor.DarkGray, "No breakpoints.", newline: true);
            return;
        }

        for (int i = 0; i < _state.Breakpoints.Count; i++)
        {
            var (f, l) = _state.Breakpoints[i];
            var location = string.IsNullOrEmpty(f) ? $"{l}" : $"{f}:{l}";
            C(ConsoleColor.DarkGray, $"  {i + 1}: ");
            C(ConsoleColor.Green, location, newline: true);
        }
    }

    private void EvalExpression(string expr)
    {
        if (string.IsNullOrWhiteSpace(expr))
        {
            Console.WriteLine("Usage: p <expression>");
            return;
        }

        try
        {
            using Lexer lexer = new(0, expr);
            Parser parser = new(rethrowErrors: true);
            var ast = parser.ParseTokenStreamCollection([lexer.GetTokenStream()]);
            var chunk = Compiler.CompileExpression((ProgramNode)ast);

            // Temporarily inject current locals into the global scope so the
            // eval expression can reference variables by name.
            var globals = Interpreter.GetGlobalScope();
            var displaced = new List<(string Name, Value Old)>();

            if (_currentVM != null)
            {
                foreach (var (name, val) in _currentVM.GetCurrentLocals())
                {
                    globals.TryGet(name, out var old);
                    displaced.Add((name, old));
                    globals.Declare(name, val);
                }
            }

            var savedHook   = _currentVM?.DebugHook;
            var savedCurrent = KiwiVM.Current;
            if (_currentVM != null) _currentVM.DebugHook = null;

            try
            {
                var evalVm = new KiwiVM(Interpreter);
                var result = evalVm.Execute(chunk);
                C(ConsoleColor.Cyan, Serializer.Serialize(result), newline: true);
            }
            finally
            {
                KiwiVM.Current = savedCurrent;
                foreach (var (name, old) in displaced)
                {
                    if (old.IsNull()) globals.Remove(name);
                    else globals.Declare(name, old);
                }
                if (_currentVM != null) _currentVM.DebugHook = savedHook;
            }
        }
        catch (KiwiError e)
        {
            Err($"Error: {e.Message}");
        }
        catch (Exception e)
        {
            Err($"Error: {e.Message}");
        }
    }

    private void ShowLocals()
    {
        if (_currentVM == null || _currentVM.FrameCount == 0)
        {
            C(ConsoleColor.DarkGray, "No active stack frame.", newline: true);
            return;
        }

        var seen    = new HashSet<string>();
        var locals  = _currentVM.GetCurrentLocals().ToList();
        var entries = locals.Count > 0 ? locals : _currentVM.GetCurrentGlobals().ToList();

        if (entries.Count == 0)
        {
            C(ConsoleColor.DarkGray, "  (no locals)", newline: true);
            return;
        }

        foreach (var (name, value) in entries)
        {
            if (seen.Add(name))
            {
                C(ConsoleColor.Green, $"  {name}");
                C(ConsoleColor.DarkGray, " = ");
                C(ConsoleColor.Cyan, Serializer.Serialize(value), newline: true);
            }
        }
    }

    private void ShowBacktrace()
    {
        if (_currentVM == null || _currentVM.FrameCount == 0)
        {
            C(ConsoleColor.DarkGray, "Empty call stack.", newline: true);
            return;
        }

        for (int i = _currentVM.FrameCount - 1; i >= 0; i--)
        {
            C(ConsoleColor.DarkGray, $"  #{i} ");
            C(ConsoleColor.Yellow, _currentVM.GetFrame(i).FormatTrace(), newline: true);
        }
    }

    private void ShowSource(string arg)
    {
        if (string.IsNullOrEmpty(_currentFile))
        {
            Console.WriteLine("No current location.");
            return;
        }

        var line = _currentLine;
        if (int.TryParse(arg, out var targetLine))
        {
            line = targetLine;
        }

        ShowSourceAround(_currentFile, line, 5);
    }

    private void ShowContext(string file, int line)
    {
        Console.WriteLine();
        C(ConsoleColor.Cyan, "=> ");
        C(ConsoleColor.White, $"{file}:{line}", newline: true);
        ShowSourceAround(file, line, 2);
    }

    private static void ShowSourceAround(string file, int line, int context)
    {
        if (string.IsNullOrEmpty(file) || !File.Exists(file))
        {
            C(ConsoleColor.DarkGray, $"  (source not available: {file})", newline: true);
            return;
        }

        try
        {
            var lines = File.ReadAllLines(file);
            var start = Math.Max(0, line - context - 1);
            var end = Math.Min(lines.Length - 1, line + context - 1);

            for (var i = start; i <= end; i++)
            {
                bool isCurrent = i + 1 == line;
                C(isCurrent ? ConsoleColor.Yellow : ConsoleColor.DarkGray, isCurrent ? "=> " : "   ");
                C(ConsoleColor.DarkGray, $"{i + 1,4}: ");
                C(isCurrent ? ConsoleColor.White : ConsoleColor.Gray, lines[i], newline: true);
            }
        }
        catch
        {
            C(ConsoleColor.DarkGray, $"  (could not read source: {file})", newline: true);
        }
    }

    private static void ShowHelp()
    {
        C(ConsoleColor.White, "kdb commands:", newline: true);
        HelpLine("r, run",          "run to next breakpoint (or end)");
        HelpLine("s, step",         "step into next statement");
        HelpLine("n, next",         "step over (skip function calls)");
        HelpLine("f, finish",       "step out of current function");
        HelpLine("b <line>",        "set breakpoint at line in current file");
        HelpLine("b <file>:<line>", "set breakpoint at file:line");
        HelpLine("d <n>",           "delete breakpoint #n");
        HelpLine("info b",          "list all breakpoints");
        HelpLine("p <expr>",        "evaluate and print expression");
        HelpLine("l, locals",       "show local variables");
        HelpLine("bt, backtrace",   "show call stack");
        HelpLine("list [line]",     "show source around current/given line");
        HelpLine("h, help",         "show this help");
        HelpLine("q, quit",         "exit debugger");
    }

    private static void HelpLine(string cmd, string desc)
    {
        C(ConsoleColor.Yellow, $"  {cmd,-16}");
        C(ConsoleColor.Gray, desc, newline: true);
    }
}
