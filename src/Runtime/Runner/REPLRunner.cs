
using System.Text;
using kiwi.Parsing;
using kiwi.Runtime.Builtin.Util;
using kiwi.Settings;
using kiwi.Tracing;
using kiwi.Tracing.Error;

namespace kiwi.Runtime.Runner;

public class REPLRunner(Interpreter interpreter) : IRunner
{
    /// <summary>
    /// A success return code. A placeholder until a smarter mechanism is implemented.
    /// </summary>
    private const int SuccessReturnCode = 0;

    /// <summary>
    /// Gets the local interpreter.
    /// </summary>
    private Interpreter Interpreter { get; } = interpreter;

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

        LoadStandardLibrary();

        StringBuilder code = new();
                
        while (true)
        {
            try
            {
                // Read a line, trim, and skip if empty.
                Console.Write("> ");
                string? input = Console.ReadLine()?.Trim();

                if (string.IsNullOrWhiteSpace(input))
                {
                    continue;
                }

                // Continue building code if the input is terminated with a '\'
                if (input.EndsWith('\\'))
                {
                    code.AppendLine(input[..^1]);
                    continue;
                }
                else if (input.Equals(".exit"))
                {
                    break;
                }
                else
                {
                    code.AppendLine(input);
                }

                // Parse and clear the string builder.
                Parser parser = new(rethrowErrors);
                using Lexer lexer = new(replId, code.ToString());

                var ast = parser.ParseTokenStreamCollection([lexer.GetTokenStream()]);
                if (parser.HasError)
                {
                    continue;
                }

                code.Clear();

                // If we have a valid AST, walk it.
                Interpreter.Interpret(ast);
            }
            catch (UnexpectedEndOfFileError)
            {
                // assume the user is writing a multi-line block
            }
            catch (KiwiError e)
            {
                ErrorHandler.PrintError(e);
            }
            catch (Exception e)
            {
                ErrorHandler.DumpCrashLog(e);
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
        Interpreter.Interpret(ast);
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