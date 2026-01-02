using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class TaskBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args, KContext context) => builtin switch
    {
        TokenName.Builtin_Task_Spawn => Spawn(context, token, args),
        TokenName.Builtin_Task_Await => Await(token, args),
        TokenName.Builtin_Task_Sleep => Sleep(token, args),
        TokenName.Builtin_Task_Status => Status(token, args),
        TokenName.Builtin_Task_Result => Result(token, args),
        TokenName.Builtin_Task_List => List(token, args),
        TokenName.Builtin_Task_Busy => Busy(token, args),
        _ => throw new FunctionUndefinedError(token, token.Text)
    };

    private static Value Spawn(KContext context, Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskSpawn, 2, args.Count);
        TypeError.ExpectLambda(token, args[0]);
        TypeError.ExpectList(token, args[1]);
        return Value.CreateInteger(TaskManager.Instance.Spawn(context, args[0].GetLambda(), args[1].GetList(), token));
    }

    private static Value Await(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskAwait, 1, args.Count);
        TypeError.ExpectInteger(token, args[0]);
        var id = args[0].GetInteger();
        return TaskManager.Instance.Await(id, token);
    }

    private static Value Sleep(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskSleep, 1, args.Count);
        var ms = ConversionOp.GetInteger(token, args[0], "Sleep requires integer ms");
        TaskManager.Instance.Sleep(ms);
        return Value.Default;
    }

    private static Value Status(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskStatus, 1, args.Count);
        TypeError.ExpectInteger(token, args[0]);
        return TaskManager.Instance.Status(args[0].GetInteger());
    }

    private static Value Result(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskResult, 1, args.Count);
        TypeError.ExpectInteger(token, args[0]);
        return TaskManager.Instance.Result(args[0].GetInteger());
    }

    private static Value List(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskList, 0, args.Count);
        return Value.CreateList(TaskManager.Instance.List());
    }

    private static Value Busy(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskBusy, 0, args.Count);
        return TaskManager.Instance.Busy();
    }
}