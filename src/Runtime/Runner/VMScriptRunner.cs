using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Settings;
using kiwi.Tracing;
using kiwi.Tracing.Error;
using kiwi.VM;

namespace kiwi.Runtime.Runner;

public class VMScriptRunner : ScriptRunner
{
    /// <summary>CLI args forwarded from Program.cs.</summary>
    public Dictionary<string, string> CliArgs { get; init; } = [];

    public override int Run(string script, List<string> args)
    {
        ExecutionPath = script;

        try
        {
            using Lexer lexer = new(script);
            return RunVMLexer(lexer);
        }
        catch (KiwiError e)
        {
            ErrorHandler.PrintError(e);
            return 1;
        }
        catch (Exception e)
        {
            ErrorHandler.DumpCrashLog(e);
            return 1;
        }

        return SuccessReturnCode;
    }

    protected int RunVMLexer(Lexer lexer)
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

            streams.Clear();

            SocketManager.Instance.Start();
            TlsSocketManager.Instance.Start();

            var programNode = (ProgramNode)ast;
            var chunk = Compiler.CompileProgram(programNode);
            if (System.Environment.GetEnvironmentVariable("KIWI_DISASM") == "1")
                Disassembler.Dump(chunk);

            var vm = new KiwiVM();
            vm.CliArgs       = CliArgs;
            vm.ExecutionPath = ExecutionPath;
            vm.ProjectRoot   = Directory.GetCurrentDirectory();
            vm.EntryPath     = string.IsNullOrEmpty(ExecutionPath)
                ? Directory.GetCurrentDirectory()
                : Path.GetFullPath(ExecutionPath);

            KiwiVM.Current = vm;
            ConfigureVM(vm);
            vm.Execute(chunk);

            AwaitTasksAndShutdown();
        }
        catch (KiwiError e)
        {
            ErrorHandler.PrintError(e);
            return 1;
        }
        catch (Exception e)
        {
            ErrorHandler.DumpCrashLog(e);
            return 1;
        }

        return SuccessReturnCode;
    }

    /// <summary>
    /// Called after the VM is created but before execution begins.
    /// Override to install debug hooks or other per-VM configuration.
    /// </summary>
    protected virtual void ConfigureVM(KiwiVM vm) { }
}
