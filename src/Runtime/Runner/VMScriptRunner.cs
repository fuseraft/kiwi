using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Settings;
using kiwi.Tracing;
using kiwi.Tracing.Error;
using kiwi.VM;

namespace kiwi.Runtime.Runner;

public class VMScriptRunner(Interpreter interpreter) : ScriptRunner(interpreter)
{
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
        }
        catch (Exception e)
        {
            ErrorHandler.DumpCrashLog(e);
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

            Interpreter.ExecutionPath = ExecutionPath;
            Interpreter.ProjectRoot   = Directory.GetCurrentDirectory();
            Interpreter.EntryPath = string.IsNullOrEmpty(ExecutionPath)
                ? Directory.GetCurrentDirectory()
                : Path.GetFullPath(ExecutionPath);

            SocketManager.Instance.Start();
            TlsSocketManager.Instance.Start();

            // Compile and execute via VM; stdlib nodes are interpreted via the tree-walker
            // (the stdlib is currently always pre-loaded through the existing Interpreter path).
            // For now we run the stdlib through the interpreter, then compile and run user code.
            var programNode = (ProgramNode)ast;
            var chunk = Compiler.CompileProgram(programNode);
            if (System.Environment.GetEnvironmentVariable("KIWI_DISASM") == "1")
                Disassembler.Dump(chunk);
            var vm = new KiwiVM(Interpreter);
            KiwiVM.Current = vm;
            ConfigureVM(vm);
            vm.Execute(chunk);

            AwaitTasksAndShutdown();
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

    /// <summary>
    /// Called after the VM is created but before execution begins.
    /// Override to install debug hooks or other per-VM configuration.
    /// </summary>
    protected virtual void ConfigureVM(KiwiVM vm) { }
}
