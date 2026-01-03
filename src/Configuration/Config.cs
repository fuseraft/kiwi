using System.Text.Json;
using kiwi.Tracing.Error;

namespace kiwi.Settings;

public class Config
{
    public bool PrintTokens { get; set; } = false;
    public bool PrintAST { get; set; } = false;
    public bool UseREPL { get; set; } = false;
    public List<string> Args { get; set; } = [];
    public List<string> Scripts { get; set; } = [];
    public bool HasScripts => Scripts.Count > 0;

    public static Config Configure(IEnumerable<string> cliArgs)
    {
        Config config = new();

        var iter = cliArgs.GetEnumerator();
        while (iter.MoveNext())
        {
            if (config.UseREPL)
            {
                break;
            }

            var current = iter.Current;

            switch (current.ToLower())
            {
                case "-i":
                case "--interactive":
                    if (!config.HasScripts)
                    {
                        config.UseREPL = true;
                        config.Scripts.Add(string.Empty);
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;

                case "-s":
                case "--settings":
                    if (!config.HasScripts)
                    {
                        PrintSettings();
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;

                case "-n":
                case "--new":
                    if (!config.HasScripts)
                    {
                        if (!iter.MoveNext())
                        {
                            throw new CliError($"Expected a filename after `{current}`.");
                        }

                        CreateNewFile(iter.Current);
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;

                case "-p":
                case "--stdlib-path":
                    if (!config.HasScripts)
                    {
                        if (!iter.MoveNext())
                        {
                            throw new CliError($"Expected a path after `{current}`.");
                        }

                        Kiwi.Settings.StandardLibrary = [new StandardLibraryPath
                        {
                            AutoLoad = true,
                            IncludeSubdirectories = true,
                            IsOverride = true,
                            Path = iter.Current
                        }];
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;

                case "-t":
                case "--tokens":
                    if (!config.HasScripts)
                    {
                        var filename = GetFilename(ref iter);
                        config.Scripts.Add(filename);
                        config.PrintTokens = true;
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;

                case "-a":
                case "--ast":
                    if (!config.HasScripts)
                    {
                        var filename = GetFilename(ref iter);
                        config.Scripts.Add(filename);
                        config.PrintAST = true;
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;

                case "-sm":
                case "--safemode":
                    if (!config.HasScripts)
                    {
                        Kiwi.Settings.SafeMode = true;
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;

                case "-ns":
                case "--no-stdlib":
                    if (!config.HasScripts)
                    {
                        Kiwi.Settings.StandardLibrary.Clear();
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;

                case "-v":
                case "--version":
                    if (!config.HasScripts)
                    {
                        PrintVersion();
                        Environment.Exit(0);
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;

                case "-h":
                case "--help":
                    if (!config.HasScripts)
                    {
                        PrintHelp();
                        Environment.Exit(0);
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;

                default:
                    if (!config.HasScripts && IsScript(ref current))
                    {
                        config.Scripts.Add(current);
                    }
                    else
                    {
                        config.Args.Add(current);
                    }
                    break;
            }
        }

        foreach (var env in Kiwi.Settings.EnvironmentVariables)
        {
            if (string.IsNullOrEmpty(env.Key))
            {
                continue;
            }

            Environment.SetEnvironmentVariable(env.Key, env.Value);
        }

        return config;
    }

    private static void PrintSettings()
    {
        Console.WriteLine(JsonSerializer.Serialize(Kiwi.Settings, KiwiJsonContext.Default.KiwiSettings));
        Environment.Exit(0);
    }

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
        Environment.Exit(0);
    }

    private static string GetFilename(ref IEnumerator<string> iter)
    {
        if (!iter.MoveNext())
        {
            throw new CliError("Expected a filename.");
        }

        var filename = iter.Current;
        if (!IsScript(ref filename))
        {
            throw new CliError($"The file does not exist: {filename}");
        }

        return filename;
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
            ("-a, --ast <input_path>", $"print syntax tree of `{Kiwi.Settings.Extensions.Primary}` file (for debugging)"),
            ("-t, --tokenize <input_path>", $"print token stream of `{Kiwi.Settings.Extensions.Primary}` file (for debugging)"),
            ("-ns, --no-stdlib", "run without standard library"),
            ("-sm, --safemode", "run in safemode"),
            ("-p, --stdlib-path", "specify an alternative standard library path"),
            ("-<key>=<value>", "pass an argument to a program as a key-value pair")
        ];

        PrintVersion();

        Console.WriteLine($"Usage: {Kiwi.Settings.Name} [--flags] <script|args>");
        Console.WriteLine("Options:");

        foreach (var (flag, description) in commands)
        {
            Console.WriteLine($"  {flag,-40}{description}");
        }
    }

    private static bool IsScript(ref string filename)
    {
        var original = filename;

        if (filename.StartsWith("~/"))
        {
            var home = Environment.GetFolderPath(Environment.SpecialFolder.UserProfile);
            var path = filename.Replace("~", home).Replace("//", "/");
            var dir = Path.GetDirectoryName(path) ?? throw new CliError($"Unable to retrieve parent path for: {path}");
            filename = Path.Combine([dir, Path.GetFileName(filename)]);
        }

        if (!Path.HasExtension(filename))
        {
            filename += Kiwi.Settings.Extensions.Primary;
        }

        if (File.Exists(filename))
        {
            return true;
        }
        else
        {
            filename = original;
        }

        return false;
    }
}