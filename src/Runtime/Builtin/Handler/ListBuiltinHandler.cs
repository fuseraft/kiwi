using kiwi.Parsing;
using kiwi.VM;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public class ListBuiltinHandler
{    
    public static Value HandleListBuiltin(KiwiVM vm, Token token, ref Value obj, TokenName op, IReadOnlyList<Value> args)
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
            vm.Context.Lambdas.TryGetValue(lambdaRef.Identifier, out var kl);
            if (kl != null) lambda = kl;
            else if (vm.Context.Functions.TryGetValue(lambdaRef.Identifier, out var kf)) lambda = kf;

            if (lambda == null)
            {
                throw new InvalidOperationError(token, $"Unrecognized lambda '{lambdaRef.Identifier}'.");
            }

            var isReturnSet = vm.CallStack.Peek().IsFlagSet(FrameFlags.Return);
            var result = Value.Default;

            switch (op)
            {
                case TokenName.Builtin_List_Sort:
                    result = LambdaSort(vm, lambda, list, token);
                    break;

                case TokenName.Builtin_List_Each:
                    result = LambdaEach(vm, lambda, list, token);
                    break;

                case TokenName.Builtin_List_Map:
                    result = LambdaMap(vm, lambda, list, token);
                    break;

                case TokenName.Builtin_List_None:
                    result = LambdaNone(vm, lambda, list, token);
                    break;

                case TokenName.Builtin_List_Filter:
                    result = LambdaFilter(vm, lambda, list, token);
                    break;

                case TokenName.Builtin_List_All:
                    result = LambdaAll(vm, lambda, list, token);

                    break;

                default:
                    break;
            }

            var frame = vm.CallStack.Peek();
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
            vm.Context.Lambdas.TryGetValue(lambdaRef.Identifier, out var kl2);
            if (kl2 != null) lambda2 = kl2;
            else if (vm.Context.Functions.TryGetValue(lambdaRef.Identifier, out var kf2)) lambda2 = kf2;

            if (lambda2 == null)
            {
                throw new InvalidOperationError(token, $"Unrecognized lambda '{lambdaRef.Identifier}'.");
            }

            return LambdaReduce(vm, lambda2, args[0], list, token);
        }

        throw new InvalidOperationError(token, "Invalid specialized list builtin invocation.");
    }


    private static Value LambdaSort(KiwiVM vm, Callable lambda, List<Value> list, Token token)
    {
        list.Sort((a, b) =>
        {
            if (BooleanOp.IsTruthy(vm.InvokeCallable(lambda, [a, b], token, "<sort>"))) return -1;
            if (BooleanOp.IsTruthy(vm.InvokeCallable(lambda, [b, a], token, "<sort>"))) return 1;
            return 0;
        });
        return Value.CreateList(list);
    }

    private static Value LambdaEach(KiwiVM vm, Callable lambda, List<Value> list, Token token)
    {
        bool twoArgs = lambda.Parameters.Count > 1;
        var buf = new Value[twoArgs ? 2 : 1];
        for (int i = 0; i < list.Count; i++)
        {
            buf[0] = list[i];
            if (twoArgs) buf[1] = Value.CreateInteger(i);
            vm.InvokeCallable(lambda, buf, token, "<each>");
        }
        return Value.Default;
    }

    private static Value LambdaNone(KiwiVM vm, Callable lambda, List<Value> list, Token token)
    {
        var filtered = LambdaFilter(vm, lambda, list, token);
        if (filtered.IsList())
            return filtered.GetList().Count == 0 ? Value.True : Value.False;
        return Value.False;
    }

    private static Value LambdaMap(KiwiVM vm, Callable lambda, List<Value> list, Token token)
    {
        var mapped = new List<Value>(list.Count);
        var buf    = new Value[1];
        foreach (var item in list)
        {
            buf[0] = item;
            mapped.Add(vm.InvokeCallable(lambda, buf, token, "<map>"));
        }
        return Value.CreateList(mapped);
    }

    private static Value LambdaReduce(KiwiVM vm, Callable lambda, Value accumulator, List<Value> list, Token token)
    {
        var acc = accumulator;
        var buf = new Value[2];
        foreach (var item in list)
        {
            var prev = acc;
            buf[0] = acc;
            buf[1] = item;
            var result = vm.InvokeCallable(lambda, buf, token, "<reduce>");
            // If the lambda returns null it likely mutated the accumulator in-place
            // (e.g. `acc["key"] = val`); keep the original reference in that case.
            acc = result.IsNull() ? prev : result;
        }
        return acc;
    }

    private static Value LambdaAll(KiwiVM vm, Callable lambda, List<Value> list, Token token)
    {
        var buf = new Value[1];
        foreach (var item in list)
        {
            buf[0] = item;
            if (!BooleanOp.IsTruthy(vm.InvokeCallable(lambda, buf, token, "<all>")))
                return Value.False;
        }
        return Value.True;
    }

    private static Value LambdaFilter(KiwiVM vm, Callable lambda, List<Value> list, Token token)
    {
        var filtered = new List<Value>();
        var buf      = new Value[1];
        foreach (var item in list)
        {
            buf[0] = item;
            if (BooleanOp.IsTruthy(vm.InvokeCallable(lambda, buf, token, "<filter>")))
                filtered.Add(item);
        }
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
        count = Math.Clamp(count, 0, lst.Count);
        return Value.CreateList(lst.GetRange(count, lst.Count - count));
    }

    private static Value ListTake(ref Value obj, int count)
    {
        var lst = obj.GetList();
        count = Math.Clamp(count, 0, lst.Count);
        return Value.CreateList(lst.GetRange(0, count));
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