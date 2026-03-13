using kiwi.Parsing;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public class ListBuiltinHandler
{    
    public static Value HandleListBuiltin(Interpreter interp, Token token, ref Value obj, TokenName op, List<Value> args)
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

        if (args.Count == 1 && args[0].IsLambda())
        {
            var arg = args[0];

            var lambdaRef = arg.GetLambda();

            if (!interp.Context.Lambdas.TryGetValue(lambdaRef.Identifier, out var lambda))
            {
                throw new InvalidOperationError(token, $"Unrecognized lambda '{lambdaRef.Identifier}'.");
            }

            var isReturnSet = interp.CallStack.Peek().IsFlagSet(FrameFlags.Return);
            var result = Value.Default;

            switch (op)
            {
                case TokenName.Builtin_List_Sort:
                    result = LambdaSort(interp, lambda, list, token);
                    break;

                case TokenName.Builtin_List_Each:
                    result = LambdaEach(interp, lambda, list, token);
                    break;

                case TokenName.Builtin_List_Map:
                    result = LambdaMap(interp, lambda, list, token);
                    break;

                case TokenName.Builtin_List_None:
                    result = LambdaNone(interp, lambda, list, token);
                    break;

                case TokenName.Builtin_List_Filter:
                    result = LambdaFilter(interp, lambda, list, token);
                    break;

                case TokenName.Builtin_List_All:
                    result = LambdaAll(interp, lambda, list, token);

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

            if (!interp.Context.Lambdas.TryGetValue(lambdaRef.Identifier, out var lambda))
            {
                throw new InvalidOperationError(token, $"Unrecognized lambda '{lambdaRef.Identifier}'.");
            }

            return LambdaReduce(interp, lambda, args[0], list, token);
        }

        throw new InvalidOperationError(token, "Invalid specialized list builtin invocation.");
    }


    private static Value LambdaSort(Interpreter interp, KLambda lambda, List<Value> list, Token token)
    {
        if (lambda.VMChunk != null)
        {
            list.Sort((a, b) =>
            {
                if (BooleanOp.IsTruthy(interp.InvokeCallable(lambda, [a, b], token, "<sort>"))) return -1;
                if (BooleanOp.IsTruthy(interp.InvokeCallable(lambda, [b, a], token, "<sort>"))) return 1;
                return 0;
            });
            return Value.CreateList(list);
        }
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
            scope.SetLocal(lhsVar, a);
            scope.SetLocal(rhsVar, b);

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

            scope.SetLocal(lhsVar, b);
            scope.SetLocal(rhsVar, a);

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

    private static Value LambdaEach(Interpreter interp, KLambda lambda, List<Value> list, Token token)
    {
        if (lambda.VMChunk != null)
        {
            for (int i = 0; i < list.Count; i++)
                interp.InvokeCallable(lambda, lambda.Parameters.Count > 1 ? [list[i], Value.CreateInteger(i)] : [list[i]], token, "<each>");
            return Value.Default;
        }
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
                scope.Declare(valueVariable, Value.Default);
            }
            else if (i == 1)
            {
                indexVariable = param.Key;
                hasIndexVariable = true;
                scope.Declare(indexVariable, Value.Default);
            }
        }

        var result = Value.Default;
        var indexValue = Value.CreateInteger(0);
        var decl = lambda.Decl;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.SetLocal(valueVariable, list[i]);

            if (hasIndexVariable)
            {
                indexValue.SetValue(i);
                scope.SetLocal(indexVariable, indexValue);
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

    private static Value LambdaNone(Interpreter interp, KLambda lambda, List<Value> list, Token token)
    {
        var filtered = LambdaFilter(interp, lambda, list, token);
        if (filtered.IsList())
            return filtered.GetList().Count == 0 ? Value.True : Value.False;
        return Value.False;
    }

    private static Value LambdaMap(Interpreter interp, KLambda lambda, List<Value> list, Token token)
    {
        if (lambda.VMChunk != null)
        {
            var mapped = new List<Value>(list.Count);
            foreach (var item in list)
                mapped.Add(interp.InvokeCallable(lambda, [item], token, "<map>"));
            return Value.CreateList(mapped);
        }
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
                scope.Declare(mapVariable, Value.Default);
            }
        }

        var decl = lambda.Decl;
        List<Value> resultList = new(list.Count);
        Value result = Value.Default;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.SetLocal(mapVariable, list[i]);

            foreach (var stmt in decl.Body)
            {
                result = interp.Interpret(stmt);
                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    frame.ClearFlag(FrameFlags.Return);
                }
            }
            resultList.Add(result);
        }

        scope.Remove(mapVariable);

        return Value.CreateList(resultList);
    }

    private static Value LambdaReduce(Interpreter interp, KLambda lambda, Value accumulator, List<Value> list, Token token)
    {
        if (lambda.VMChunk != null)
        {
            var acc = accumulator;
            foreach (var item in list)
                acc = interp.InvokeCallable(lambda, [acc, item], token, "<reduce>");
            return acc;
        }
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
                scope.Declare(accumVariable, accumulator);
            }
            else if (i == 1)
            {
                valueVariable = param.Key;
                scope.Declare(valueVariable, Value.Default);
            }
        }

        var decl = lambda.Decl;
        Value result;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.SetLocal(valueVariable, list[i]);

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

    private static Value LambdaAll(Interpreter interp, KLambda lambda, List<Value> list, Token token)
    {
        if (lambda.VMChunk != null)
        {
            foreach (var item in list)
                if (!BooleanOp.IsTruthy(interp.InvokeCallable(lambda, [item], token, "<all>")))
                    return Value.False;
            return Value.True;
        }
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
                scope.Declare(valueVariable, Value.Default);
            }
            else if (i == 1)
            {
                indexVariable = param.Key;
                hasIndexVariable = true;
                scope.Declare(indexVariable, Value.Default);
            }
        }

        var result = Value.Default;
        var indexValue = Value.CreateInteger(0);
        var decl = lambda.Decl;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.SetLocal(valueVariable, list[i]);

            if (hasIndexVariable)
            {
                indexValue.SetValue(i);
                scope.SetLocal(indexVariable, indexValue);
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

    private static Value LambdaFilter(Interpreter interp, KLambda lambda, List<Value> list, Token token)
    {
        if (lambda.VMChunk != null)
        {
            var filtered = new List<Value>();
            foreach (var item in list)
                if (BooleanOp.IsTruthy(interp.InvokeCallable(lambda, [item], token, "<filter>")))
                    filtered.Add(item);
            return Value.CreateList(filtered);
        }
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
                scope.Declare(valueVariable, Value.Default);
            }
            else if (i == 1)
            {
                indexVariable = param.Key;
                hasIndexVariable = true;
                scope.Declare(indexVariable, Value.Default);
            }
        }

        var result = Value.Default;
        var indexValue = Value.CreateInteger(0);
        var decl = lambda.Decl;
        List<Value> resultList = [];

        for (var i = 0; i < list.Count; ++i)
        {
            scope.SetLocal(valueVariable, list[i]);

            if (hasIndexVariable)
            {
                indexValue.SetValue(i);
                scope.SetLocal(indexVariable, indexValue);
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