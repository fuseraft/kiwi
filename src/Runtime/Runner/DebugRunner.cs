using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Runtime.Builtin.Util;
using kiwi.Runtime.Debugger;
using kiwi.Settings;
using kiwi.Tracing;
using kiwi.Tracing.Error;
using kiwi.Typing;
using static kiwi.Parsing.AST.ASTTracer;

namespace kiwi.Runtime.Runner;

public class DebugRunner(Interpreter interpreter) : ScriptRunner(interpreter)
{
    private static readonly HashSet<ASTNodeType> StatementNodes =
    [
        ASTNodeType.Assignment,
        ASTNodeType.ConstAssignment,
        ASTNodeType.IndexAssignment,
        ASTNodeType.MemberAssignment,
        ASTNodeType.PackAssignment,
        ASTNodeType.If,
        ASTNodeType.Case,
        ASTNodeType.WhileLoop,
        ASTNodeType.ForLoop,
        ASTNodeType.RepeatLoop,
        ASTNodeType.Do,
        ASTNodeType.FunctionCall,
        ASTNodeType.MethodCall,
        ASTNodeType.LambdaCall,
        ASTNodeType.Function,
        ASTNodeType.Struct,
        ASTNodeType.Return,
        ASTNodeType.Break,
        ASTNodeType.Next,
        ASTNodeType.Exit,
        ASTNodeType.Print,
        ASTNodeType.PrintXy,
        ASTNodeType.Throw,
        ASTNodeType.Try,
        ASTNodeType.Include,
        ASTNodeType.Import,
        ASTNodeType.Require,
        ASTNodeType.Emit,
        ASTNodeType.On,
        ASTNodeType.Once,
        ASTNodeType.Off,
        ASTNodeType.Export,
        ASTNodeType.Package,
        ASTNodeType.Eval,
    ];

    private readonly DebugState _state = new();
    private readonly HashSet<string> _stdlibDirs = [];
    private ASTNode? _currentNode;

    public override int Run(string script, List<string> args)
    {
        ComputeStdlibDirs();

        Console.WriteLine("kdb - Kiwi Debugger");
        Console.WriteLine($"Debugging: {script}");
        Console.WriteLine("Type 'h' for help, 's' to step, 'r' to run.");
        Console.WriteLine();

        _state.Mode = DebugMode.StepIn;
        Interpreter.DebugHook = OnDebugHook;

        return base.Run(script, args);
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

    private void OnDebugHook(ASTNode node)
    {
        if (!StatementNodes.Contains(node.Type))
        {
            return;
        }

        var fileId = node.Token.Span.File;
        var line = node.Token.Span.Line;
        var file = FileRegistry.Instance.GetFilePath(fileId);

        if (string.IsNullOrEmpty(file) || IsStdlibFile(file))
        {
            return;
        }

        if (file == _state.LastFile && line == _state.LastLine)
        {
            return;
        }

        var depth = Interpreter.CallStack.Count;

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
        _currentNode = node;

        ShowContext(file, line);
        RunDebugREPL();
    }

    private void RunDebugREPL()
    {
        while (true)
        {
            Console.Write("(kdb) ");
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
                    _state.StepDepth = Interpreter.CallStack.Count;
                    return;

                case "n":
                case "next":
                    _state.Mode = DebugMode.StepOver;
                    _state.StepDepth = Interpreter.CallStack.Count;
                    return;

                case "f":
                case "finish":
                    _state.Mode = DebugMode.StepOut;
                    _state.StepDepth = Interpreter.CallStack.Count;
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
                    Console.WriteLine($"Unknown command: '{cmd}'. Type 'h' for help.");
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
        Console.WriteLine($"Breakpoint {_state.Breakpoints.Count} at {location}");
    }

    private void HandleDeleteBreakpoint(string arg)
    {
        if (!int.TryParse(arg, out var index))
        {
            Console.WriteLine("Usage: d <n>  (where n is the breakpoint number from 'info b')");
            return;
        }

        if (_state.RemoveBreakpoint(index))
        {
            Console.WriteLine($"Deleted breakpoint {index}");
        }
        else
        {
            Console.WriteLine($"No breakpoint #{index}");
        }
    }

    private void ListBreakpoints()
    {
        if (_state.Breakpoints.Count == 0)
        {
            Console.WriteLine("No breakpoints.");
            return;
        }

        for (int i = 0; i < _state.Breakpoints.Count; i++)
        {
            var (f, l) = _state.Breakpoints[i];
            var location = string.IsNullOrEmpty(f) ? $"{l}" : $"{f}:{l}";
            Console.WriteLine($"  {i + 1}: {location}");
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

            var savedHook = Interpreter.DebugHook;
            Interpreter.DebugHook = null;

            try
            {
                var result = Interpreter.Interpret(ast);
                Console.WriteLine(Serializer.Serialize(result));
            }
            finally
            {
                Interpreter.DebugHook = savedHook;
            }
        }
        catch (KiwiError e)
        {
            Console.WriteLine($"Error: {e.Message}");
        }
        catch (Exception e)
        {
            Console.WriteLine($"Error: {e.Message}");
        }
    }

    private void ShowLocals()
    {
        if (Interpreter.CallStack.Count == 0)
        {
            Console.WriteLine("No active stack frame.");
            return;
        }

        var scope = Interpreter.CallStack.Peek().Scope;
        var seen = new HashSet<string>();

        foreach (var (name, value) in scope.GetAllBindings())
        {
            if (seen.Add(name))
            {
                Console.WriteLine($"  {name} = {Serializer.Serialize(value)}");
            }
        }
    }

    private void ShowBacktrace()
    {
        if (Interpreter.CallStack.Count == 0)
        {
            Console.WriteLine("Empty call stack.");
            return;
        }

        foreach (var frame in Interpreter.CallStack.Reverse())
        {
            Console.WriteLine(frame.FormatTraceLine());
        }
    }

    private void ShowSource(string arg)
    {
        if (_currentNode == null)
        {
            Console.WriteLine("No current location.");
            return;
        }

        var fileId = _currentNode.Token.Span.File;
        var file = FileRegistry.Instance.GetFilePath(fileId);
        var line = _currentNode.Token.Span.Line;

        if (int.TryParse(arg, out var targetLine))
        {
            line = targetLine;
        }

        ShowSourceAround(file, line, 5);
    }

    private void ShowContext(string file, int line)
    {
        Console.WriteLine();
        Console.WriteLine($"=> {file}:{line}");
        ShowSourceAround(file, line, 2);
    }

    private static void ShowSourceAround(string file, int line, int context)
    {
        if (string.IsNullOrEmpty(file) || !File.Exists(file))
        {
            Console.WriteLine($"  (source not available: {file})");
            return;
        }

        try
        {
            var lines = File.ReadAllLines(file);
            var start = Math.Max(0, line - context - 1);
            var end = Math.Min(lines.Length - 1, line + context - 1);

            for (var i = start; i <= end; i++)
            {
                var marker = i + 1 == line ? "=>" : "  ";
                Console.WriteLine($"{marker} {i + 1,4}: {lines[i]}");
            }
        }
        catch
        {
            Console.WriteLine($"  (could not read source: {file})");
        }
    }

    private static void ShowHelp()
    {
        Console.WriteLine("kdb commands:");
        Console.WriteLine("  r, run          run to next breakpoint (or end)");
        Console.WriteLine("  s, step         step into next statement");
        Console.WriteLine("  n, next         step over (skip function calls)");
        Console.WriteLine("  f, finish       step out of current function");
        Console.WriteLine("  b <line>        set breakpoint at line in current file");
        Console.WriteLine("  b <file>:<line> set breakpoint at file:line");
        Console.WriteLine("  d <n>           delete breakpoint #n");
        Console.WriteLine("  info b          list all breakpoints");
        Console.WriteLine("  p <expr>        evaluate and print expression");
        Console.WriteLine("  l, locals       show local variables");
        Console.WriteLine("  bt, backtrace   show call stack");
        Console.WriteLine("  list [line]     show source around current/given line");
        Console.WriteLine("  h, help         show this help");
        Console.WriteLine("  q, quit         exit debugger");
    }
}
