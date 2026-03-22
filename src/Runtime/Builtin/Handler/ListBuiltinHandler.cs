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

        if (args.Count == 1 && args[0].IsString() && op == TokenName.Builtin_List_GroupByField)
        {
            return ListGroupByField(token, list, args[0].GetString());
        }

        if (args.Count == 1 && args[0].IsLambda())
        {
            var arg = args[0];

            var lambdaRef = arg.GetLambda();

            Callable? lambda = null;
            interp.Context.Lambdas.TryGetValue(lambdaRef.Identifier, out var kl);
            if (kl != null) lambda = kl;
            else if (interp.Context.Functions.TryGetValue(lambdaRef.Identifier, out var kf)) lambda = kf;

            if (lambda == null)
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

            Callable? lambda2 = null;
            interp.Context.Lambdas.TryGetValue(lambdaRef.Identifier, out var kl2);
            if (kl2 != null) lambda2 = kl2;
            else if (interp.Context.Functions.TryGetValue(lambdaRef.Identifier, out var kf2)) lambda2 = kf2;

            if (lambda2 == null)
            {
                throw new InvalidOperationError(token, $"Unrecognized lambda '{lambdaRef.Identifier}'.");
            }

            return LambdaReduce(interp, lambda2, args[0], list, token);
        }

        throw new InvalidOperationError(token, "Invalid specialized list builtin invocation.");
    }


    private static Value LambdaSort(Interpreter interp, Callable lambda, List<Value> list, Token token)
    {
        list.Sort((a, b) =>
        {
            if (BooleanOp.IsTruthy(interp.InvokeCallable(lambda, [a, b], token, "<sort>"))) return -1;
            if (BooleanOp.IsTruthy(interp.InvokeCallable(lambda, [b, a], token, "<sort>"))) return 1;
            return 0;
        });
        return Value.CreateList(list);
    }

    private static Value LambdaEach(Interpreter interp, Callable lambda, List<Value> list, Token token)
    {
        for (int i = 0; i < list.Count; i++)
            interp.InvokeCallable(lambda, lambda.Parameters.Count > 1 ? [list[i], Value.CreateInteger(i)] : [list[i]], token, "<each>");
        return Value.Default;
    }

    private static Value LambdaNone(Interpreter interp, Callable lambda, List<Value> list, Token token)
    {
        var filtered = LambdaFilter(interp, lambda, list, token);
        if (filtered.IsList())
            return filtered.GetList().Count == 0 ? Value.True : Value.False;
        return Value.False;
    }

    private static Value LambdaMap(Interpreter interp, Callable lambda, List<Value> list, Token token)
    {
        var mapped = new List<Value>(list.Count);
        foreach (var item in list)
            mapped.Add(interp.InvokeCallable(lambda, [item], token, "<map>"));
        return Value.CreateList(mapped);
    }

    private static Value LambdaReduce(Interpreter interp, Callable lambda, Value accumulator, List<Value> list, Token token)
    {
        var acc = accumulator;
        foreach (var item in list)
        {
            var prev = acc;
            var result = interp.InvokeCallable(lambda, [acc, item], token, "<reduce>");
            // If the lambda returns null it likely mutated the accumulator in-place
            // (e.g. `acc["key"] = val`); keep the original reference in that case.
            acc = result.IsNull() ? prev : result;
        }
        return acc;
    }

    private static Value LambdaAll(Interpreter interp, Callable lambda, List<Value> list, Token token)
    {
        foreach (var item in list)
            if (!BooleanOp.IsTruthy(interp.InvokeCallable(lambda, [item], token, "<all>")))
                return Value.False;
        return Value.True;
    }

    private static Value LambdaFilter(Interpreter interp, Callable lambda, List<Value> list, Token token)
    {
        var filtered = new List<Value>();
        foreach (var item in list)
            if (BooleanOp.IsTruthy(interp.InvokeCallable(lambda, [item], token, "<filter>")))
                filtered.Add(item);
        return Value.CreateList(filtered);
    }

    /// <summary>
    /// Groups a list of hashmaps by a string field in O(n) with no Kiwi lambda calls.
    /// Returns a hashmap whose keys are the distinct field values (as strings) and whose
    /// values are lists of the matching rows. Insertion order of keys is preserved.
    /// </summary>
    private static Value ListGroupByField(Token token, List<Value> list, string field)
    {
        var fieldKey = Value.CreateString(field);
        // groups[key] = C# list — built directly, no copy-on-read overhead.
        var groups     = new Dictionary<string, List<Value>>(StringComparer.Ordinal);
        var keyOrder   = new List<string>();

        foreach (var row in list)
        {
            if (!row.IsHashmap()) continue;
            var hash = row.GetHashmap();
            if (!hash.TryGetValue(fieldKey, out var fieldVal)) continue;
            var key = fieldVal.Value_?.ToString() ?? string.Empty;
            if (!groups.TryGetValue(key, out var bucket))
            {
                bucket = new List<Value>();
                groups[key] = bucket;
                keyOrder.Add(key);
            }
            bucket.Add(row);
        }

        var result = new Dictionary<Value, Value>(keyOrder.Count);
        foreach (var k in keyOrder)
            result[Value.CreateString(k)] = Value.CreateList(groups[k]);
        return Value.CreateHashmap(result);
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