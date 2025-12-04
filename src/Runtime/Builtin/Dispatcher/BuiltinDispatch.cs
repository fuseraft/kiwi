using kiwi.Runtime.Builtin.Handler;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Dispatcher;

public struct BuiltinDispatch
{
    public static Value Execute(Token token, TokenName builtin, Value v, List<Value> args)
    {
        if (CoreBuiltin.IsBuiltin(builtin))
        {
            return CoreBuiltinHandler.Execute(token, builtin, v, args);
        }

        throw new FunctionUndefinedError(token, token.Text);
    }

    public static Value Execute(Token token, TokenName builtin, List<Value> args, Dictionary<string, string> cliArgs)
    {
        if (FileIOBuiltin.IsBuiltin(builtin))
        {
            return FileIOBuiltinHandler.Execute(token, builtin, args);
        }
        else if (TimeBuiltin.IsBuiltin(builtin))
        {
            return TimeBuiltinHandler.Execute(token, builtin, args);
        }
        else if (MathBuiltin.IsBuiltin(builtin))
        {
            return MathBuiltinHandler.Execute(token, builtin, args);
        }
        else if (EnvBuiltin.IsBuiltin(builtin))
        {
            return EnvBuiltinHandler.Execute(token, builtin, args, cliArgs);
        }
        else if (EncoderBuiltin.IsBuiltin(builtin))
        {
            return EncoderBuiltinHandler.Execute(token, builtin, args);
        }
        else if (ConsoleBuiltin.IsBuiltin(builtin))
        {
            return ConsoleBuiltinHandler.Execute(token, builtin, args);
        }
        else if (SysBuiltin.IsBuiltin(builtin))
        {
            return SysBuiltinHandler.Execute(token, builtin, args);
        }
        else if (SerializerBuiltin.IsBuiltin(builtin))
        {
            return SerializerBuiltinHandler.Execute(token, builtin, args);
        }
        else if (StdInBuiltin.IsBuiltin(builtin))
        {
            return StdinBuiltinHandler.Execute(token, builtin, args);
        }
        else if (HttpBuiltin.IsBuiltin(builtin))
        {
            // return HttpBuiltinHandler::execute(token, builtin, args);
        }
        else if (LoggingBuiltin.IsBuiltin(builtin))
        {
            // return LoggingBuiltinHandler::execute(token, builtin, args);
        }

        throw new FunctionUndefinedError(token, token.Text);
    }

    /*
    public static Value Execute(ref FFIManager ffi, Token token, TokenName builtin, List<Value> args)
    {
        return FFIBuiltinHandler::execute(ffi, token, builtin, args);
    }

    public static Value Execute(ref SocketManager sockmgr, Token token, TokenName builtin, List<Value> args)
    {
        return NetBuiltinHandler::execute(sockmgr, token, builtin, args);
    }

    public static Value Execute(ref TaskManager taskmgr, Token token, TokenName builtin, List<Value> args)
    {
        return TaskBuiltinHandler::execute(taskmgr, token, builtin, args);
    }
    */
}