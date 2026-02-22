using kiwi.Parsing;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public class ListBuiltinHandler
{    
    public static Value HandleListBuiltin(Token token, ref Value obj, TokenName op, List<Value> args)
    {
        if (!obj.IsList())
        {
            throw new InvalidOperationError(token, "Expected a list for specialized list builtin.");
        }

        var list = obj.GetList();

        switch (op)
        {
            case TokenName.Builtin_List_Max:
                return ListMax(token, list);

            case TokenName.Builtin_List_Min:
                return ListMin(token, list);

            case TokenName.Builtin_List_Sort:
                // perform a simple sort, otherwise expect a lambda.
                if (args.Count == 0)
                {
                    return ListSort(list);
                }
                break;

            case TokenName.Builtin_List_Sum:
                return ListSum(list);

            default:
                break;
        }

        if (args.Count == 1 && args[0].IsInteger())
        {
            switch (op)
            {
                case TokenName.Builtin_List_Skip:
                    return ListSkip(ref obj, (int)args[0].GetInteger());
                case TokenName.Builtin_List_Take:
                    return ListTake(ref obj, (int)args[0].GetInteger());
            }
        }

        var interp = Interpreter.Current ?? throw new RuntimeError(token, $"{op}", []);

        if (args.Count == 1 && args[0].IsLambda())
        {
            var arg = args[0];

            var lambdaRef = arg.GetLambda();

            if (!interp.Context.HasLambda(lambdaRef.Identifier))
            {
                throw new InvalidOperationError(token, $"Unrecognized lambda '{lambdaRef.Identifier}'.");
            }

            var lambda = interp.Context.Lambdas[lambdaRef.Identifier];
            var isReturnSet = interp.CallStack.Peek().IsFlagSet(FrameFlags.Return);
            var result = Value.Default;

            switch (op)
            {
                case TokenName.Builtin_List_Sort:
                    result = LambdaSort(interp, lambda, list);
                    break;

                case TokenName.Builtin_List_Each:
                    result = LambdaEach(interp, lambda, list);
                    break;

                case TokenName.Builtin_List_Map:
                    result = LambdaMap(interp, lambda, list);
                    break;

                case TokenName.Builtin_List_None:
                    result = LambdaNone(interp, lambda, list);
                    break;

                case TokenName.Builtin_List_Filter:
                    result = LambdaFilter(interp, lambda, list);
                    break;

                case TokenName.Builtin_List_All:
                    result = LambdaAll(interp, lambda, list);
                    break;

                default:
                    break;
            }

            var frame = interp.CallStack.Peek();
            if (!isReturnSet && frame.IsFlagSet(FrameFlags.Return) && frame.ReturnValue != null)
            {
                if (!BooleanOp.IsSame(frame.ReturnValue, result))
                {
                    frame.ReturnValue = result;
                }
            }

            return result;
        }
        
        if (args.Count == 2 && op == TokenName.Builtin_List_Reduce)
        {
            var arg = args[1];

            if (!arg.IsLambda())
            {
                throw new InvalidOperationError(token, "Expected a lambda in specialized list builtin.");
            }
            var lambdaRef = arg.GetLambda();

            if (!interp.Context.HasLambda(lambdaRef.Identifier))
            {
                throw new InvalidOperationError(token, $"Unrecognized lambda '{lambdaRef.Identifier}'.");
            }

            var lambda = interp.Context.Lambdas[lambdaRef.Identifier];

            return LambdaReduce(interp, lambda, args[0], list);
        }

        throw new InvalidOperationError(token, "Invalid specialized list builtin invocation.");
    }


    private static Value LambdaSort(Interpreter interp, KLambda lambda, List<Value> list)
    {
        var frame = interp.CallStack.Peek();
        var scope = frame.Scope;

        if (lambda.Parameters.Count != 2)
        {
            return Value.CreateList(list);
        }

        var lhsVar = lambda.Parameters[0].Key;
        var rhsVar = lambda.Parameters[1].Key;

        scope.Declare(lhsVar, Value.Default);
        scope.Declare(rhsVar, Value.Default);

        var decl = lambda.Decl;

        list.Sort((a, b) =>
        {
            scope.Assign(lhsVar, a);
            scope.Assign(rhsVar, b);

            Value result = Value.Default;

            foreach (var stmt in decl.Body)
            {
                result = interp.Interpret(stmt);
                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    frame.ClearFlag(FrameFlags.Return);
                }
            }

            bool isLess = BooleanOp.IsTruthy(result);

            if (isLess) return -1;

            scope.Assign(lhsVar, b);
            scope.Assign(rhsVar, a);

            result = Value.Default;

            foreach (var stmt in decl.Body)
            {
                result = interp.Interpret(stmt);
                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    frame.ClearFlag(FrameFlags.Return);
                }
            }

            bool isGreater = BooleanOp.IsTruthy(result);

            if (isGreater)
            {
                return 1;
            }

            return 0;
        });

        scope.Remove(lhsVar);
        scope.Remove(rhsVar);

        return Value.CreateList(list);
    }

    private static Value LambdaEach(Interpreter interp, KLambda lambda, List<Value> list)
    {
        var defaultParameters = lambda.DefaultParameters;
        var scope = interp.CallStack.Peek().Scope;

        var valueVariable = string.Empty;
        var indexVariable = string.Empty;
        var hasIndexVariable = false;

        if (lambda.Parameters.Count == 0)
        {
            return Value.Default;
        }

        for (var i = 0; i < lambda.Parameters.Count; ++i)
        {
            var param = lambda.Parameters[i];
            if (i == 0)
            {
                valueVariable = param.Key;
                scope.Assign(valueVariable, Value.Default);
            }
            else if (i == 1)
            {
                indexVariable = param.Key;
                hasIndexVariable = true;
                scope.Assign(indexVariable, Value.Default);
            }
        }

        var result = Value.Default;
        var indexValue = Value.Default;
        var decl = lambda.Decl;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.Assign(valueVariable, list[i]);

            if (hasIndexVariable)
            {
                indexValue.SetValue(i);
                scope.Assign(indexVariable, indexValue);
            }

            foreach (var stmt in decl.Body)
            {
                result = interp.Interpret(stmt);
            }
        }

        scope.Remove(valueVariable);
        if (hasIndexVariable)
        {
            scope.Remove(indexVariable);
        }

        return result;
    }

    private static Value LambdaNone(Interpreter interp, KLambda lambda, List<Value> list)
    {
        var filtered = LambdaFilter(interp, lambda, list);
        var noneFound = Value.False;

        if (filtered.IsList())
        {
            var isEmpty = filtered.GetList().Count == 0;
            noneFound.SetValue(isEmpty);
        }

        return noneFound;
    }

    private static Value LambdaMap(Interpreter interp, KLambda lambda, List<Value> list)
    {
        var defaultParameters = lambda.DefaultParameters;
        var frame = interp.CallStack.Peek();
        var scope = frame.Scope;

        var mapVariable = string.Empty;

        if (lambda.Parameters.Count == 0)
        {
            return Value.CreateList(list);
        }

        for (var i = 0; i < lambda.Parameters.Count; ++i)
        {
            var param = lambda.Parameters[i];
            if (i == 0)
            {
                mapVariable = param.Key;
                scope.Assign(mapVariable, Value.Default);
            }
        }

        var decl = lambda.Decl;
        List<Value> resultList = [];
        Value result = Value.Default;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.Assign(mapVariable, list[i]);

            foreach (var stmt in decl.Body)
            {
                result = interp.Interpret(stmt);
                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    frame.ClearFlag(FrameFlags.Return);
                }
                resultList.Add(result);
            }
        }

        scope.Remove(mapVariable);

        return Value.CreateList(resultList);
    }

    private static Value LambdaReduce(Interpreter interp, KLambda lambda, Value accumulator, List<Value> list)
    {
        var defaultParameters = lambda.DefaultParameters;
        var scope = interp.CallStack.Peek().Scope;

        var accumVariable = string.Empty;
        var valueVariable = string.Empty;

        if (lambda.Parameters.Count != 2)
        {
            return accumulator;
        }

        for (var i = 0; i < lambda.Parameters.Count; ++i)
        {
            var param = lambda.Parameters[i];
            if (i == 0)
            {
                accumVariable = param.Key;
                scope.Assign(accumVariable, accumulator);
            }
            else if (i == 1)
            {
                valueVariable = param.Key;
                scope.Assign(valueVariable, Value.Default);
            }
        }

        var decl = lambda.Decl;
        Value result;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.Assign(valueVariable, list[i]);

            foreach (var stmt in decl.Body)
            {
                result = interp.Interpret(stmt);
            }
        }

        result = scope.GetBinding(accumVariable);

        scope.Remove(accumVariable);
        scope.Remove(valueVariable);

        return result;
    }

    private static Value LambdaAll(Interpreter interp, KLambda lambda, List<Value> list)
    {
        var defaultParameters = lambda.DefaultParameters;
        var scope = interp.CallStack.Peek().Scope;

        var valueVariable = string.Empty;
        var indexVariable = string.Empty;
        var hasIndexVariable = false;

        var listSize = list.Count;
        var newListSize = 0;

        for (var i = 0; i < lambda.Parameters.Count; ++i)
        {
            var param = lambda.Parameters[i];
            if (i == 0)
            {
                valueVariable = param.Key;
                scope.Assign(valueVariable, Value.Default);
            }
            else if (i == 1)
            {
                indexVariable = param.Key;
                hasIndexVariable = true;
                scope.Assign(indexVariable, Value.Default);
            }
        }

        var result = Value.Default;
        var indexValue = Value.Default;
        var decl = lambda.Decl;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.Assign(valueVariable, list[i]);

            if (hasIndexVariable)
            {
                indexValue.SetValue(i);
                scope.Assign(indexVariable, indexValue);
            }

            foreach (var stmt in decl.Body)
            {
                result = interp.Interpret(stmt);

                if (BooleanOp.IsTruthy(result))
                {
                    ++newListSize;
                }
            }
        }

        scope.Remove(valueVariable);
        if (hasIndexVariable)
        {
            scope.Remove(indexVariable);
        }

        return Value.CreateBoolean(newListSize == listSize);
    }

    private static Value LambdaFilter(Interpreter interp, KLambda lambda, List<Value> list)
    {
        var defaultParameters = lambda.DefaultParameters;
        var scope = interp.CallStack.Peek().Scope;

        var valueVariable = string.Empty;
        var indexVariable = string.Empty;
        var hasIndexVariable = false;

        for (var i = 0; i < lambda.Parameters.Count; ++i)
        {
            var param = lambda.Parameters[i];
            if (i == 0)
            {
                valueVariable = param.Key;
                scope.Assign(valueVariable, Value.Default);
            }
            else if (i == 1)
            {
                indexVariable = param.Key;
                hasIndexVariable = true;
                scope.Assign(indexVariable, Value.Default);
            }
        }

        var result = Value.Default;
        var indexValue = Value.Default;
        var decl = lambda.Decl;
        List<Value> resultList = [];

        for (var i = 0; i < list.Count; ++i)
        {
            scope.Assign(valueVariable, list[i]);

            if (hasIndexVariable)
            {
                indexValue.SetValue(i);
                scope.Assign(indexVariable, indexValue);
            }

            foreach (var stmt in decl.Body)
            {
                result = interp.Interpret(stmt);

                if (BooleanOp.IsTruthy(result))
                {
                    resultList.Add(list[i]);
                }
            }
        }

        scope.Remove(valueVariable);
        if (hasIndexVariable)
        {
            scope.Remove(indexVariable);
        }

        return Value.CreateList(resultList);
    }

    private static Value ListSkip(ref Value obj, int count)
    {
        var lst = obj.GetList();

        try
        {
            return Value.CreateList([.. lst.Skip(count)]);
        }
        catch {}

        return Value.CreateList();
    }

    private static Value ListTake(ref Value obj, int count)
    {
        var lst = obj.GetList();

        try
        {
            return Value.CreateList([.. lst.Take(count)]);
        }
        catch {}

        return Value.CreateList();
    }

    private static Value ListSum(List<Value> list)
    {
        var sum = 0D;
        var isFloatResult = false;

        foreach (var val in list)
        {
            if (val.IsInteger())
            {
                sum += val.GetInteger();
            }
            else if (val.IsFloat())
            {
                sum += val.GetFloat();
                isFloatResult = true;
            }
        }

        if (isFloatResult)
        {
            return Value.CreateFloat(sum);
        }

        return Value.CreateInteger((long)sum);
    }

    private static Value ListMin(Token token, List<Value> list)
    {
        if (list.Count == 0)
        {
            return Value.CreateNull();
        }

        var minValue = list[0];

        for (var i = 0; i < list.Count; i++)
        {
            var val = list[i];

            if (ComparisonOp.GetLtResult(ref val, ref minValue))
            {
                minValue = val;
            }
        }

        return minValue;
    }

    private static Value ListMax(Token token, List<Value> list)
    {
        if (list.Count == 0)
        {
            return Value.CreateNull();
        }

        var maxValue = list[0];

        for (var i = 0; i < list.Count; i++)
        {
            var val = list[i];

            if (ComparisonOp.GetGtResult(ref val, ref maxValue))
            {
                maxValue = val;
            }
        }

        return maxValue;
    }

    private static Value ListSort(List<Value> list)
    {
        list.Sort();
        return Value.CreateList(list);
    }
}