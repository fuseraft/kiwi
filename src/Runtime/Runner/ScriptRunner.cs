
using kiwi.Parsing;
using kiwi.Runtime.Builtin.Util;
using kiwi.Settings;
using kiwi.Tracing;
using kiwi.Tracing.Error;

namespace kiwi.Runtime.Runner;
public class ScriptRunner(Interpreter interpreter) : IRunner
{
    /// <summary>
    /// A success return code. A placeholder until a smarter mechanism is implemented.
    /// </summary>
    protected const int SuccessReturnCode = 0;

    /// <summary>
    /// Gets the local interpreter.
    /// </summary>
    protected Interpreter Interpreter { get; } = interpreter;

    /// <summary>
    /// Gets or sets a flag indicating whether the standard library has been loaded.
    /// </summary>
    protected bool StandardLibraryLoaded { get; set; } = false;

    /// <summary>
    /// Gets or sets the path of execution.
    /// </summary>
    private string ExecutionPath { get; set; } = string.Empty;

    /// <summary>
    /// Runs a given script as the entrypoint to the program.
    /// </summary>
    /// <param name="script">The script.</param>
    /// <param name="args">The arguments.</param>
    /// <returns>Returns <c>0</c> for now.</returns>
    public virtual int Run(string script, List<string> args)
    {
        int res = SuccessReturnCode;

        ExecutionPath = script;

        try
        {
            using Lexer lexer = new(script);
            res = RunLexer(lexer);
        }
        catch (KiwiError e)
        {
            ErrorHandler.PrintError(e);
        }
        catch (Exception e)
        {
            ErrorHandler.DumpCrashLog(e);
        }

        return res;
    }

    protected int RunLexer(Lexer lexer)
    {
        try
        {
            Parser parser = new();

            List<TokenStream> streams = [];
            LoadStandardLibrary(ref streams);

            streams.Add(lexer.GetTokenStream());
            var ast = parser.ParseTokenStreamCollection(streams);

            if (parser.HasError)
            {
                return 1;
            }

            Interpreter.ExecutionPath = ExecutionPath;
            Interpreter.Interpret(ast);
        }
        catch (KiwiError e)
        {
            ErrorHandler.PrintError(e);
        }
        catch (Exception e)
        {
            ErrorHandler.DumpCrashLog(e);
        }

        return SuccessReturnCode;
    }

    protected void LoadStandardLibrary(ref List<TokenStream> streams)
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