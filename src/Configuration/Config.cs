using System.Text.Json;
using kiwi.Runtime.Builtin.Util;
using kiwi.Tracing.Error;

namespace kiwi.Settings;

/// <summary>
/// What the command line asks <c>kiwi</c> to do.
/// </summary>
public enum CliMode
{
    /// <summary>Run a script, or piped code, or start the REPL when stdin is a terminal.</summary>
    Run,
    Repl,
    Execute,
    Check,
    Ast,
    Tokens,
    Debug,
    Help,
    Version,
    Settings,
    New
}

/// <summary>
/// The parsed command line: <c>kiwi [options] [script] [arguments]</c>.
/// <para>
/// Options are read until the first positional argument or <c>--</c>. In the default mode that
/// argument is the script, if it names a file; everything after it is passed to the script
/// unchanged. <c>-key=value</c> arguments before the script are passed to it too.
/// </para>
/// </summary>
public class Config
{
    public CliMode Mode { get; private set; } = CliMode.Run;

    /// <summary>The script to run, check or print; for <c>--new</c>, the file to create.</summary>
    public string Script { get; private set; } = string.Empty;

    public string? ExecuteCode { get; private set; }

    public List<string> Args { get; } = [];

    public bool HasScript => !string.IsNullOrEmpty(Script);

    // The option that chose the mode, for error messages.
    private string? _modeOption;
    private string? _stdlibOption;
    private string? _noStdlibOption;
    private readonly List<string> _stdlibPaths = [];

    public static Config Configure(IReadOnlyList<string> cliArgs)
    {
        Config config = new();
        var readingOptions = true;
        var i = 0;

        while (i < cliArgs.Count)
        {
            var arg = cliArgs[i++];

            if (readingOptions && arg == "--")
            {
                readingOptions = false;
            }
            else if (readingOptions && IsKeyValue(arg))
            {
                config.Args.Add(arg);
            }
            else if (readingOptions && arg.Length > 1 && arg.StartsWith('-'))
            {
                config.ReadOption(arg, cliArgs, ref i);
            }
            else
            {
                // The first positional argument ends the options.
                readingOptions = false;
                config.AddPositional(arg);
            }
        }

        config.Finish();
        return config;
    }

    /// <summary>
    /// Runs a command that finishes without running code (help, version, settings, new).
    /// </summary>
    /// <returns><c>true</c> if the mode was one of these.</returns>
    public bool RunCommand()
    {
        switch (Mode)
        {
            case CliMode.Help:
                PrintHelp();
                return true;
            case CliMode.Version:
                PrintVersion();
                return true;
            case CliMode.Settings:
                Console.WriteLine(JsonSerializer.Serialize(Kiwi.Settings, KiwiJsonContext.Default.KiwiSettings));
                return true;
            case CliMode.New:
                CreateNewFile(Script);
                return true;
            default:
                return false;
        }
    }

    private void ReadOption(string option, IReadOnlyList<string> cliArgs, ref int i)
    {
        switch (option.ToLowerInvariant())
        {
            case "-h":
            case "--help":
                SetMode(CliMode.Help, option);
                break;

            case "-v":
            case "--version":
                SetMode(CliMode.Version, option);
                break;

            case "-s":
            case "--settings":
                SetMode(CliMode.Settings, option);
                break;

            case "-n":
            case "--new":
                SetMode(CliMode.New, option);
                Script = ReadValue(option, "a filename", cliArgs, ref i);
                break;

            case "-i":
            case "--interactive":
                SetMode(CliMode.Repl, option);
                break;

            case "-e":
            case "--execute":
                SetMode(CliMode.Execute, option);
                ExecuteCode = ReadValue(option, "a code string", cliArgs, ref i);
                break;

            case "-c":
            case "--check":
                SetMode(CliMode.Check, option);
                Script = ReadScript(option, cliArgs, ref i);
                break;

            case "-a":
            case "--ast":
                SetMode(CliMode.Ast, option);
                Script = ReadScript(option, cliArgs, ref i);
                break;

            case "-t":
            case "--tokens":
                SetMode(CliMode.Tokens, option);
                Script = ReadScript(option, cliArgs, ref i);
                break;

            case "-d":
            case "--debug":
                SetMode(CliMode.Debug, option);
                Script = ReadScript(option, cliArgs, ref i);
                break;

            case "-p":
            case "--stdlib-path":
                {
                    // Several locations are separated by `;`, which, unlike `:`, can't be part
                    // of a Windows path such as C:\kiwi\lib.
                    var value = ReadValue(option, "a path", cliArgs, ref i);
                    var paths = value.Split(';', StringSplitOptions.TrimEntries | StringSplitOptions.RemoveEmptyEntries);
                    if (paths.Length == 0)
                    {
                        throw new CliError($"Expected a path after `{option}`.");
                    }

                    _stdlibPaths.AddRange(paths);
                    _stdlibOption = option;
                }
                break;

            case "-ns":
            case "--no-stdlib":
                _noStdlibOption = option;
                break;

            case "-sm":
            case "--safemode":
                Kiwi.Settings.SafeMode = true;
                break;

            case "-cd":
            case "--crash-dump":
                if (string.IsNullOrEmpty(Kiwi.Settings.CrashDumpPath))
                {
                    Kiwi.Settings.CrashDumpPath = "./kiwi_crash.log";
                }
                break;

            default:
                throw new CliError($"Unknown option: {option}");
        }
    }

    private void AddPositional(string arg)
    {
        // Only the default mode takes its script as a positional argument, and only the first.
        if (Mode != CliMode.Run || HasScript || Args.Any(a => !IsKeyValue(a)))
        {
            Args.Add(arg);
        }
        else if (IsScript(ref arg))
        {
            Script = arg;
        }
        else if (IsScriptName(arg))
        {
            // Otherwise a mistyped script name would become an argument to the REPL or to piped
            // code, and nothing would say the file wasn't found.
            throw new CliError($"The file does not exist: {arg}");
        }
        else
        {
            Args.Add(arg);
        }
    }

    private void SetMode(CliMode mode, string option)
    {
        // `--help` wins over anything else on the line.
        if (Mode == CliMode.Help)
        {
            return;
        }

        if (_modeOption != null && mode != CliMode.Help)
        {
            throw new CliError(_modeOption.Equals(option, StringComparison.OrdinalIgnoreCase)
                ? $"`{option}` can only be given once."
                : $"`{option}` can't be used with `{_modeOption}`.");
        }

        Mode = mode;
        _modeOption = option;
    }

    private void Finish()
    {
        if (_noStdlibOption != null && _stdlibOption != null)
        {
            throw new CliError($"`{_noStdlibOption}` can't be used with `{_stdlibOption}`.");
        }

        if (_noStdlibOption != null)
        {
            Kiwi.Settings.StandardLibrary.Clear();
        }
        else if (_stdlibPaths.Count > 0)
        {
            Kiwi.Settings.StandardLibrary = [.. _stdlibPaths.Select(path => new StandardLibraryPath
            {
                AutoLoad = true,
                IncludeSubdirectories = true,
                IsOverride = true,
                Path = path
            })];
        }

        foreach (var env in Kiwi.Settings.EnvironmentVariables)
        {
            if (string.IsNullOrEmpty(env.Key))
            {
                continue;
            }

            Environment.SetEnvironmentVariable(env.Key, env.Value);
        }
    }

    private static string ReadValue(string option, string what, IReadOnlyList<string> cliArgs, ref int i)
    {
        if (i >= cliArgs.Count)
        {
            throw new CliError($"Expected {what} after `{option}`.");
        }

        return cliArgs[i++];
    }

    private static string ReadScript(string option, IReadOnlyList<string> cliArgs, ref int i)
    {
        var filename = ReadValue(option, "a filename", cliArgs, ref i);
        if (!IsScript(ref filename))
        {
            throw new CliError($"The file does not exist: {filename}");
        }

        return filename;
    }

    // `-key=value`, which is passed to the script rather than read as an option.
    private static bool IsKeyValue(string arg) => arg.Length > 1 && arg.StartsWith('-') && arg.Contains('=');

    private static void CreateNewFile(string filename)
    {
        if (IsScript(ref filename))
        {
            throw new CliError($"The script already exists: {filename}");
        }

        if (!Path.HasExtension(filename))
        {
            filename += Kiwi.Settings.Extensions.Primary;
        }

        using var fio = File.Create(filename);

        Console.WriteLine($"Created {filename}");
    }

    private static void PrintVersion()
    {
        Console.WriteLine($"{Kiwi.Settings.Name} {Kiwi.Settings.Version}");
    }

    private static void PrintHelp()
    {
        List<(string, string)> commands =
        [
            ("-h, --help", "print this message"),
            ("-v, --version", "print current version"),
            ("-s, --settings", $"print {Kiwi.Settings.Name} settings"),
            ("-n, --new <filename>", $"create a `{Kiwi.Settings.Extensions.Primary}` file"),
            ("-i, --interactive", "run in interactive mode"),
            ("-e, --execute <code>", "execute a string of code"),
            ("-c, --check <input_path>", $"check `{Kiwi.Settings.Extensions.Primary}` file for syntax errors"),
            ("-a, --ast <input_path>", $"print syntax tree of `{Kiwi.Settings.Extensions.Primary}` file"),
            ("-t, --tokens <input_path>", $"print tokens of `{Kiwi.Settings.Extensions.Primary}` file"),
            ("-ns, --no-stdlib", "run without standard library"),
            ("-sm, --safemode", "run in safemode"),
            ("-p, --stdlib-path <paths>", "specify alternate stdlib paths, separated by `;`"),
            ("-d, --debug <input_path>", "run script in the kdb debugger"),
            ("-cd, --crash-dump", $"write errors to kiwi_crash.log"),
            ("-<key>=<value>", "pass an argument as a key-value pair"),
            ("--", "end the options; the rest are the script and its arguments")
        ];

        PrintVersion();

        Console.WriteLine($"Usage: {Kiwi.Settings.Name} [options] [script] [arguments]");
        Console.WriteLine("Options:");

        foreach (var (flag, description) in commands)
        {
            Console.WriteLine($"  {flag,-30}{description}");
        }
    }

    private static bool IsScriptName(string filename) =>
        Kiwi.Settings.Extensions.Recognized.Any(ext => filename.EndsWith(ext, StringComparison.OrdinalIgnoreCase));

    private static bool IsScript(ref string filename)
    {
        var path = FileUtil.ExpandHome(filename);

        if (!Path.HasExtension(path))
        {
            path += Kiwi.Settings.Extensions.Primary;
        }

        if (!File.Exists(path))
        {
            return false;
        }

        filename = path;
        return true;
    }
}
