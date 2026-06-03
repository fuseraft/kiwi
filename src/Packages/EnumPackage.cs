using kiwi.Parsing;
using kiwi.Runtime;
using kiwi.Tracing.Error;
using kiwi.Typing;
using System.Collections.Generic;
using System.Linq;

namespace kiwi.Packages;

public static class EnumPackage
{
    public static void RegisterBuiltins()
    {
        // dispatch handled in TypeBuiltins + KiwiVM.IsEnum path
    }



    public static Value Size(KStruct enm)
    {
        return Value.CreateInteger(enm.StaticVariables.Count);
    }

    public static Value Keys(KStruct enm)
    {
        var keys = enm.StaticVariables.Keys.Select(k => Value.CreateString(k)).ToList();
        return Value.CreateList(keys);
    }

    public static Value Values(KStruct enm)
    {
        var vals = enm.StaticVariables.Values.ToList();
        return Value.CreateList(vals);
    }

    public static Value ToHashmap(KStruct enm)
    {
        var dict = new Dictionary<Value, Value>();
        foreach (var kv in enm.StaticVariables)
            dict[Value.CreateString(kv.Key)] = kv.Value;
        return Value.CreateHashmap(dict);
    }

    public static Value Get(KStruct enm, Value key, Value? def = null)
    {
        string k = key.GetString();
        if (enm.StaticVariables.TryGetValue(k, out var v))
            return v;
        if (enm.StaticVariables.TryGetValue("@@" + k, out v))
            return v;
        if (def != null)
            return def;
        throw new Exception($"Enum member '{k}' not found");
    }
}
