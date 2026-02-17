using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public class ObjectBuiltinHandler
{
    public static Value Handle(Token token, TokenName builtin, InstanceRef obj, string baseStruct, List<Value> args)
    {
        var instVars = obj.InstanceVariables;
        var interp = Interpreter.Current ?? throw new RuntimeError(token, "Could not get runtime interpreter.", []);

        switch (builtin)
        {
            case TokenName.Builtin_Core_Clone:
                return Value.CreateObject(new InstanceRef
                {
                    StructName = obj.StructName,
                    InstanceVariables = obj.InstanceVariables.ToDictionary()
                });

            case TokenName.Builtin_Core_HasKey:
                ParameterCountMismatchError.Check(token, CoreBuiltin.HasKey, 1, args.Count);
                ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.HasKey, 0, args[0]);
                return Value.CreateBoolean(obj.HasVariable(args[0].GetString()));

            case TokenName.Builtin_Core_Get:
                ParameterCountMismatchError.CheckRange(token, CoreBuiltin.Get, 1, 2, args.Count);
                ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.Get, 0, args[0]);
                var getKey = $"@{args[0].GetString()}";
                var defaultValue = args.Count == 2 ? args[1] : Value.CreateNull();
                return obj.HasVariable(getKey) ? obj.InstanceVariables[getKey] : defaultValue;

            case TokenName.Builtin_Core_IsA:
                ParameterCountMismatchError.Check(token, CoreBuiltin.IsA, 1, args.Count);
                string? type = null;

                if (args[0].IsStruct())
                {
                    type = args[0].GetStruct().Identifier;
                }
                else if (args[0].IsString())
                {
                    type = args[0].GetString();
                }

                if (string.IsNullOrEmpty(type))
                {
                    throw new ParameterTypeMismatchError(token, CoreBuiltin.IsA, 0, args[0].Type, [Typing.ValueType.Struct, Typing.ValueType.String]);
                }

                List<string> hierarchy = [obj.StructName];
                if (!string.IsNullOrEmpty(baseStruct))
                {
                    var baseStructName = interp.Context.Structs[baseStruct].Name;
                    while (!string.IsNullOrEmpty(baseStructName))
                    {
                        hierarchy.Add(baseStructName);
                        if (!string.IsNullOrEmpty(interp.Context.Structs[baseStructName].BaseStruct))
                        {
                            baseStructName = interp.Context.Structs[baseStructName].BaseStruct;
                            continue;
                        }
                        break;
                    }
                }

                return Value.CreateBoolean(hierarchy.Contains(type));

            case TokenName.Builtin_Core_Keys:
                ParameterCountMismatchError.Check(token, CoreBuiltin.Keys, 0, args.Count);
                List<Value> keys = [.. obj.InstanceVariables.Keys.Select(key => Value.CreateString(key[1..]))];
                return Value.CreateList(keys);

            case TokenName.Builtin_Core_Set:
                ParameterCountMismatchError.Check(token, CoreBuiltin.Set, 2, args.Count);
                ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.Set, 0, args[0]);
                var setKey = $"@{args[0].GetString()}";
                if (!obj.HasVariable(setKey))
                {
                    throw new VariableUndefinedError(token, args[0].GetString());
                }
                obj.InstanceVariables[setKey] = args[1];
                break;

            case TokenName.Builtin_Core_Values:
                ParameterCountMismatchError.Check(token, CoreBuiltin.Values, 0, args.Count);
                List<Value> values = [.. obj.InstanceVariables.Values];
                return Value.CreateList(values);
        }

        return Value.Default;
    }
}