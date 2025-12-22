using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class TaskBuiltinHandler
{
    public static Value Execute(TaskManager mgr, Token token, TokenName builtin, List<Value> args, KContext context) => builtin switch
    {
        TokenName.Builtin_Task_Spawn => Spawn(context, mgr, token, args),
        TokenName.Builtin_Task_Await => Await(mgr, token, args),
        TokenName.Builtin_Task_Sleep => Sleep(mgr, token, args),
        TokenName.Builtin_Task_Status => Status(mgr, token, args),
        TokenName.Builtin_Task_Result => Result(mgr, token, args),
        TokenName.Builtin_Task_List => List(mgr, token, args),
        TokenName.Builtin_Task_Busy => Busy(mgr, token, args),
        _ => throw new FunctionUndefinedError(token, token.Text)
    };

    private static Value Spawn(KContext context, TaskManager mgr, Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskSpawn, 2, args.Count);
        TypeError.ExpectLambda(token, args[0]);
        TypeError.ExpectList(token, args[1]);

        return Value.CreateInteger(mgr.Spawn(context, args[0].GetLambda(), args[1].GetList(), token));
    }

    private static Value Await(TaskManager mgr, Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskAwait, 1, args.Count);
        var id = ConversionOp.GetInteger(token, args[0], "Await requires integer id");
        return mgr.Await(id, token);
    }

    private static Value Sleep(TaskManager mgr, Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskSleep, 1, args.Count);
        var ms = ConversionOp.GetInteger(token, args[0], "Sleep requires integer ms");
        mgr.Sleep(ms);
        return Value.Default;
    }

    private static Value Status(TaskManager mgr, Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskStatus, 1, args.Count);
        TypeError.ExpectInteger(token, args[0]);
        return mgr.Status(args[0].GetInteger());
    }

    private static Value Result(TaskManager mgr, Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskResult, 1, args.Count);
        TypeError.ExpectInteger(token, args[0]);
        return mgr.Result(args[0].GetInteger());
    }

    private static Value List(TaskManager mgr, Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskList, 0, args.Count);
        return Value.CreateList(mgr.List());
    }

    private static Value Busy(TaskManager mgr, Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TaskBuiltin.TaskBusy, 0, args.Count);
        return mgr.Busy();
    }
}