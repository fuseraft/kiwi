using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class EnvBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args, Dictionary<string, string> cliArgs)
    {
        return builtin switch
        {
            TokenName.Builtin_Env_GetArgv => GetArgv(token, args, cliArgs),
            TokenName.Builtin_Env_GetXarg => GetXarg(token, args, cliArgs),
            TokenName.Builtin_Env_GetAll => GetAll(token, args),
            TokenName.Builtin_Env_GetEnvironmentVariable => GetEnvironmentVariable(token, args),
            TokenName.Builtin_Env_SetEnvironmentVariable => SetEnvironmentVariable(token, args),
            TokenName.Builtin_Env_Kiwi => GetBinPath(token, args),
            TokenName.Builtin_Env_OS => OS(token, args),
            TokenName.Builtin_Env_User => User(token, args),
            TokenName.Builtin_Env_UserDomain => UserDomain(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value GetArgv(Token token, List<Value> args, Dictionary<string, string> cliArgs)
    {
        ParameterCountMismatchError.Check(token, EnvBuiltin.GetArgv, 0, args.Count);

        Dictionary<Value, Value> argv = [];

        foreach (var pair in cliArgs)
        {
            argv[Value.CreateString(pair.Key)] = Value.CreateString(pair.Value);
        }

        return Value.CreateHashmap(argv);
    }

    private static Value GetXarg(Token token, List<Value> args, Dictionary<string, string> cliArgs)
    {
        ParameterCountMismatchError.Check(token, EnvBuiltin.GetXarg, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, EnvBuiltin.GetXarg, 0, args[0]);

        var xargName = args[0].GetString();

        foreach (var pair in cliArgs)
        {
            if (pair.Key.Equals(xargName))
            {
                return Value.CreateString(pair.Value);
            }
        }

        return Value.EmptyString;
    }

    private static Value OS(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EnvBuiltin.OS, 0, args.Count);

        return Value.CreateString(System.Runtime.InteropServices.RuntimeInformation.OSDescription);
    }

    private static Value User(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EnvBuiltin.User, 0, args.Count);

        return Value.CreateString(Environment.UserName);
    }

    private static Value UserDomain(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EnvBuiltin.UserDomain, 0, args.Count);

        return Value.CreateString(Environment.UserDomainName);
    }

    private static Value GetAll(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EnvBuiltin.GetAll, 0, args.Count);

        var envVars = Environment.GetEnvironmentVariables()
            .Cast<System.Collections.DictionaryEntry>()
            .ToDictionary(
                entry => Value.CreateString((string)entry.Key), 
                entry => Value.CreateString(entry.Value ?? string.Empty)
            );
        return Value.CreateHashmap(envVars);
    }

    private static Value GetBinPath(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EnvBuiltin.Kiwi, 0, args.Count);

        var exePath = Environment.ProcessPath ?? throw new FileSystemError(token, "Could not get executable path.");
        return Value.CreateString(exePath);
    }

    private static Value GetEnvironmentVariable(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EnvBuiltin.GetEnvironmentVariable, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, EnvBuiltin.GetEnvironmentVariable, 0, args[0]);

        var env = Environment.GetEnvironmentVariable(args[0].GetString()) ?? string.Empty;
        return Value.CreateString(env);
    }

    private static Value SetEnvironmentVariable(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, EnvBuiltin.SetEnvironmentVariable, 2, args.Count);

        ParameterTypeMismatchError.ExpectString(token, EnvBuiltin.SetEnvironmentVariable, 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, EnvBuiltin.SetEnvironmentVariable, 1, args[1]);

        Environment.SetEnvironmentVariable(args[0].GetString(), args[1].GetString());
        return Value.Default;
    }
}