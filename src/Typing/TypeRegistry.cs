using kiwi.Parsing;
using kiwi.Tracing.Error;

namespace kiwi.Typing;

public static class TypeRegistry
{
    private static Dictionary<int, Typing.ValueType> registeredPrimitives = []; 
    private static Dictionary<string, int> registeredTypes = [];
    private static Dictionary<int, string> registeredTypeNames = [];
    private static bool defaultsRegistered = false;

    static TypeRegistry()
    {
        RegisterDefaults();
    }

    private static void RegisterDefaults()
    {
        if (defaultsRegistered)
        {
            return;
        }

        // any is 0
        RegisterType("any");
        RegisterType("integer");
        RegisterType("float");
        RegisterType("boolean");
        RegisterType("date");
        RegisterType("hashmap");
        RegisterType("lambda");
        RegisterType("string");
        RegisterType("list");
        RegisterType("bytes");
        RegisterType("object");
        RegisterType("pointer");
        RegisterType("none");

        // map primitive types to value type enum
        registeredPrimitives.Add(GetType("integer"), ValueType.Integer);
        registeredPrimitives.Add(GetType("float"), ValueType.Float);
        registeredPrimitives.Add(GetType("boolean"), ValueType.Boolean);
        registeredPrimitives.Add(GetType("date"), ValueType.Date);
        registeredPrimitives.Add(GetType("hashmap"), ValueType.Hashmap);
        registeredPrimitives.Add(GetType("string"), ValueType.String);
        registeredPrimitives.Add(GetType("list"), ValueType.List);
        registeredPrimitives.Add(GetType("bytes"), ValueType.Bytes);
    }

    public static bool TryGetPrimitiveType(string name, out int type)
    {
        if (IsRegistered(name))
        {
            type = GetType(name);
            return true;
        }

        type = -1;
        return false;
    }

    public static bool IsPrimitive(int type) => registeredPrimitives.ContainsKey(type);

    public static Typing.ValueType GetValueType(int type) => registeredPrimitives[type];

    public static string GetTypeName(int type) => registeredTypeNames[type];

    public static string GetTypeName(Value v)
    {
        var typeName = string.Empty;

        typeName = v.Type switch
        {
            Typing.ValueType.Boolean => "boolean",
            Typing.ValueType.Float => "float",
            Typing.ValueType.Hashmap => "hashmap",
            Typing.ValueType.Integer => "integer",
            Typing.ValueType.Date => "date",
            Typing.ValueType.Lambda => "lambda",
            Typing.ValueType.List => "list",
            Typing.ValueType.None => "none",
            Typing.ValueType.String => "string",
            Typing.ValueType.Pointer => "pointer",
            Typing.ValueType.Bytes => "bytes",
            _ => string.Empty,
        };
        
        if (string.IsNullOrEmpty(typeName) && v.IsObject())
        {
            typeName = v.GetObject().StructName;
        }

        return typeName;
    }

    public static int GetType(Token t, Value v)
    {
        var typeName = GetTypeName(v);
        return GetType(t, typeName);
    }

    public static int GetType(Token t, string typeName)
    {
        if (!registeredTypes.ContainsKey(typeName))
        {
            throw new TypeError(t, $"Undefined type `{typeName}`");
        }

        return registeredTypes[typeName];
    }

    public static bool IsRegistered(string typeName) => registeredTypes.ContainsKey(typeName);

    public static int GetType(string typeName)
    {
        if (!registeredTypes.ContainsKey(typeName))
        {
            RegisterType(typeName);
        }

        return registeredTypes[typeName];
    }

    public static void RegisterType(string typeName)
    {
        var type = registeredTypes.Count;
        registeredTypes.Add(typeName, type);
        registeredTypeNames.Add(type, typeName);
    }
}