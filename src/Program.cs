
using kiwi.Settings;
using kiwi.Tracing;
using kiwi.Runtime.Runner;
using kiwi.Tracing.Error;
using System.Text;

namespace kiwi;
public class Program
{
    public static int Main(string[] args)
    {
        var exitCode = 1;
        
        try
        {   
            Console.OutputEncoding = new UTF8Encoding(encoderShouldEmitUTF8Identifier: false);
            Console.InputEncoding  = Encoding.UTF8;

            if (System.Diagnostics.Debugger.IsAttached)
            {
                args = [.. Kiwi.Settings.Debug.CommandLineArguments];
            }

            var config = Config.Configure(args);
            var runner = GetRunner(config);

            if (config.HasScripts)
            {
                foreach (var script in config.Scripts)
                {
                    var _ = runner.Run(script, config.Args);

                    if (exitCode != 0 && exitCode != _)
                    {
                        exitCode = _;
                    }
                }
            }
            else
            {
                var _ = runner.Run(string.Empty, config.Args);

                if (exitCode != 0 && exitCode != _)
                {
                    exitCode = _;
                }
            }
        }
        catch (CliError e)
        {
            Console.WriteLine(e.Message);
        }
        catch (Exception e)
        {
            ErrorHandler.DumpCrashLog(e);
        }

        return exitCode;
    }

    private static IRunner GetRunner(Config config)
    {
        if (config.PrintAST)
        {
            return new ASTPrinter();
        }
        else if (config.PrintTokens)
        {
            return new TokenPrinter();
        }
        else if (config.CheckSyntax)
        {
            return new SyntaxChecker();
        }
        else if (config.UseREPL)
        {
            return new REPLRunner(new()
            {
                CliArgs = ParseKeyValueArgs(config.Args),
            });
        }
        else if (config.ExecuteCode != null)
        {
            return new CodeRunner(config.ExecuteCode, new()
            {
                CliArgs = ParseKeyValueArgs(config.Args),
            });
        }
        else if (config.UseDebugger)
        {
            return new DebugRunner(new()
            {
                CliArgs = ParseKeyValueArgs(config.Args),
            });
        }
        else if (config.HasScripts)
        {
            return new VMScriptRunner(new()
            {
                CliArgs = ParseKeyValueArgs(config.Args),
            });
        }

        if (!Console.IsInputRedirected)
        {
            return new REPLRunner(new()
            {
                CliArgs = ParseKeyValueArgs(config.Args),
            });
        }

        return new StdInRunner(new()
        {
            CliArgs = ParseKeyValueArgs(config.Args),
        });
    }

    /// <summary>
    /// A helper method for parsing command-line arguments.
    /// </summary>
    /// <param name="args">Command-line arguments.</param>
    /// <returns>A dictionary.</returns>
    private static Dictionary<string, string> ParseKeyValueArgs(List<string> args)
    {
        Dictionary<string, string> result = [];

        int i = 0;
        while (i < args.Count)
        {
            var arg = args[i];
            string argWithoutPrefix;

            if (arg.StartsWith("--"))
            {
                argWithoutPrefix = arg[2..];
            }
            else if (arg.StartsWith('-') || arg.StartsWith('/'))
            {
                argWithoutPrefix = arg[1..];
            }
            else
            {
                // Positional: key and value are both the arg itself
                result[arg] = arg;
                i++;
                continue;
            }

            // Split on the first '='
            var parts = argWithoutPrefix.Split('=', 2, StringSplitOptions.RemoveEmptyEntries);

            if (parts.Length == 2)
            {
                result[parts[0]] = parts[1];
            }
            else if (!string.IsNullOrWhiteSpace(argWithoutPrefix))
            {
                // Lookahead: consume next token as value when it isn't another flag or a URL.
                // This enables `--key value` syntax in addition to `--key=value`.
                bool consumedNext = false;
                if (i + 1 < args.Count)
                {
                    var next = args[i + 1];
                    // A "flag" must start with '-'. Paths starting with '/' are values, not flags.
                    bool nextIsFlag = next.StartsWith('-');
                    bool nextIsUrl  = next.StartsWith("http://",  StringComparison.OrdinalIgnoreCase)
                                   || next.StartsWith("https://", StringComparison.OrdinalIgnoreCase);

                    if (!nextIsFlag && !nextIsUrl)
                    {
                        result[argWithoutPrefix] = next;
                        i++;
                        consumedNext = true;
                    }
                }

                if (!consumedNext)
                {
                    result[argWithoutPrefix] = "true";
                }
            }

            i++;
        }

        return result;
    }
}