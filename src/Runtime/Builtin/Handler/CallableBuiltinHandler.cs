using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;
public static class CallableBuiltinHandler
{
    public static Value Execute(Interpreter interp, Token token, TokenName builtin, Callable callable, string callableName, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Callable_Call => Call(interp, token, callable, callableName, args),
            TokenName.Builtin_Callable_Parameters => Parameters(token, callable, args),
            TokenName.Builtin_Callable_Returns => Returns(token, callable, args),
            TokenName.Builtin_Callable_ToLambda => ToLambda(interp, token, callable, callableName, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Call(Interpreter interp, Token token, Callable callable, string callableName, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CallableBuiltin.Call, 1, args.Count);
        TypeError.ExpectList(token, args[0]);
        return interp.InvokeCallable(callable, args[0].GetList(), token, callableName);
    }

    private static Value Parameters(Token token, Callable callable, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CallableBuiltin.Parameters, 0, args.Count);
        List<Value> parameters = [];
        int pos = 0;
        foreach (var kvp in callable.Parameters)
        {
            Dictionary<Value, Value> param = [];
            param[Value.CreateString("name")] = Value.CreateString(kvp.Key);
            param[Value.CreateString("default_value")] = kvp.Value;

            if (callable.TypeHints.TryGetValue(kvp.Key, out var hint))
            {
                param[Value.CreateString("type")] = Value.CreateString(TypeRegistry.GetTypeName(hint));
            }

            param[Value.CreateString("position")] = Value.CreateInteger(pos++);

            parameters.Add(Value.CreateHashmap(param));
        }
        return Value.CreateList(parameters);
    }

    private static Value Returns(Token token, Callable callable, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CallableBuiltin.Returns, 0, args.Count);
        return Value.CreateString(TypeRegistry.GetTypeName(callable.ReturnTypeHint));
    }

    private static Value ToLambda(Interpreter interp, Token token, Callable callable, string callableName, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CallableBuiltin.ToLambda, 0, args.Count);
        
        if (callable is KFunction func)
        {
            return interp.FuncToLambda(func);
        }
        else if (callable is KLambda)
        {
            return Value.CreateLambda(new LambdaRef { Identifier = callableName });
        }
        
        throw new InvalidOperationError(token, "Expected a function or a lambda as a callable.");
    }
}