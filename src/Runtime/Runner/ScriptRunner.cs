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
    protected string ExecutionPath { get; set; } = string.Empty;

    /// <summary>
    /// Runs a given script as the entrypoint to the program.
    /// </summary>
    /// <param name="script">The script.</param>
    /// <param name="args">The arguments.</param>
    /// <returns>Returns <c>0</c> for now.</returns>
    public virtual int Run(string script, List<string> args) => SuccessReturnCode;

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

    protected void AwaitTasksAndShutdown()
    {
        if (TaskManager.Instance.Busy().GetBoolean() == true)
        {
            while (TaskManager.Instance.Busy().GetBoolean())
            {
                var activeCount = 0;
                foreach (var task in TaskManager.Instance.List())
                {
                    var status = TaskManager.Instance.Status(task.GetInteger()).GetString();
                    if (!(status is "Faulted" or "Completed"))
                    {
                        activeCount++;
                    }
                }

                if (activeCount == 0)
                {
                    break;
                }

                TaskManager.Instance.Sleep(10);
            }
        }

        SocketManager.Instance.Stop();
        TlsSocketManager.Instance.Stop();
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