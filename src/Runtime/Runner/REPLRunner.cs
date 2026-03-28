
using System.Text;
using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Runtime.Builtin.Util;
using kiwi.Settings;
using kiwi.Tracing;
using kiwi.Tracing.Error;
using kiwi.Typing;
using kiwi.VM;

namespace kiwi.Runtime.Runner;

public class REPLRunner : IRunner
{
    /// <summary>
    /// A success return code. A placeholder until a smarter mechanism is implemented.
    /// </summary>
    private const int SuccessReturnCode = 0;

    /// <summary>CLI args forwarded from Program.cs.</summary>
    public Dictionary<string, string> CliArgs { get; init; } = [];

    private LineEditor Editor { get; } = new();
    private KiwiVM? _vm;

    /// <summary>
    /// Gets or sets a flag indicating whether the standard library has been loaded.
    /// </summary>
    private bool StandardLibraryLoaded { get; set; } = false;

    /// <summary>
    /// Runs the interpreter in REPL mode.
    /// </summary>
    /// <param name="script">The script.</param>
    /// <param name="args">The arguments.</param>
    /// <returns>Returns <c>0</c> for now.</returns>
    public int Run(string script, List<string> args)
    {
        RunREPL();
        return SuccessReturnCode;
    }

    private void RunREPL()
    {
        const int replId = 0;
        const bool rethrowErrors = true;

        Console.WriteLine($"{Kiwi.Settings.Name} {Kiwi.Settings.Version} - type 'exit' or press Ctrl+C to quit.");

        LoadStandardLibrary();

        StringBuilder code = new();
        bool inContinuation = false;

        while (true)
        {
            try
            {
                string? input;
                try
                {
                    input = Editor.ReadLine(inContinuation ? ".. " : ">> ");
                }
                catch (EndOfStreamException)
                {
                    break; // Ctrl+D on empty line
                }

                // Ctrl+D / EOF from piped input
                if (input == null)
                    break;

                input = input.Trim();

                if (string.IsNullOrWhiteSpace(input))
                {
                    continue;
                }

                // Explicit line continuation with backslash
                if (input.EndsWith('\\'))
                {
                    code.AppendLine(input[..^1]);
                    inContinuation = true;
                    continue;
                }

                if (input is ".exit" or "exit")
                {
                    break;
                }

                code.AppendLine(input);

                // Parse
                Parser parser = new(rethrowErrors);
                using Lexer lexer = new(replId, code.ToString());

                var ast = parser.ParseTokenStreamCollection([lexer.GetTokenStream()]);
                if (parser.HasError)
                {
                    code.Clear();
                    inContinuation = false;
                    continue;
                }

                code.Clear();
                inContinuation = false;

                // Execute and auto-print non-null results
                var chunk = Compiler.CompileExpression((ProgramNode)ast);
                var vm = _vm != null ? new KiwiVM(_vm) : new KiwiVM();
                KiwiVM.Current = vm;
                var result = vm.Execute(chunk);
                if (!result.IsNull())
                {
                    Console.ForegroundColor = ConsoleColor.Cyan;
                    Console.Write("=> ");
                    Console.ResetColor();
                    Console.WriteLine(Serializer.Serialize(result, wrapStrings: true));
                }
            }
            catch (UnexpectedEndOfFileError)
            {
                // Implicit multi-line block and wait for more input
                inContinuation = true;
            }
            catch (KiwiError e)
            {
                ErrorHandler.PrintError(e);
                code.Clear();
                inContinuation = false;
            }
            catch (Exception e)
            {
                ErrorHandler.DumpCrashLog(e);
                code.Clear();
                inContinuation = false;
            }
        }
    }

    private void LoadStandardLibrary()
    {
        if (StandardLibraryLoaded)
        {
            return;
        }

        // Load the standard library.
        Parser parser = new();
        List<TokenStream> streams = [];
        LoadStandardLibrary(ref streams);

        var ast = parser.ParseTokenStreamCollection(streams);
        var chunk = Compiler.CompileProgram((ProgramNode)ast);
        _vm = new KiwiVM();
        _vm.CliArgs = CliArgs;
        KiwiVM.Current = _vm;
        _vm.Execute(chunk);
    }

    private void LoadStandardLibrary(ref List<TokenStream> streams)
    {
        if (StandardLibraryLoaded)
        {
            return;
        }

        List<string> paths = [];
        var exePath = Environment.ProcessPath ?? string.Empty;

        foreach (var library in Kiwi.Settings.StandardLibrary)
        {
            if (!library.AutoLoad)
            {
                continue;
            }

            var libraryPath = library.IsOverride ? FileUtil.ExpandPath(library.Path) : Path.Combine(Path.GetDirectoryName(exePath) ?? string.Empty, library.Path);
            libraryPath = Path.GetFullPath(libraryPath);

            if (Directory.Exists(libraryPath) && library.IncludeSubdirectories)
            {
                foreach (var path in Directory.EnumerateFiles(libraryPath, "*.*"))
                {
                    if (IsRecognizedScript(path))
                    {
                        paths.Add(path);
                    }
                }
            }
            else if (IsRecognizedScript(libraryPath))
            {
                paths.Add(libraryPath);
            }
        }

        paths.Sort();
        paths.Reverse();

        foreach (var path in paths)
        {
            using Lexer lexer = new(path);
            streams.Add(lexer.GetTokenStream());
        }

        StandardLibraryLoaded = true;
    }

    private static bool IsRecognizedScript(string path)
    {
        if (!File.Exists(path))
        {
            return false;
        }

        var ext = Path.GetExtension(path);
        return Kiwi.Settings.Extensions.Recognized.Contains(ext);
    }
}
