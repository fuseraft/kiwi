using kiwi.Parsing;
using kiwi.Tracing.Error;

namespace kiwi.Typing;

public static class TypeRegistry
{
    private static Dictionary<int, Typing.ValueType> registeredPrimitives = []; 
    // Concurrent: struct and enum types can be registered from task threads.
    private static readonly System.Collections.Concurrent.ConcurrentDictionary<string, int> registeredTypes = new();
    private static readonly System.Collections.Concurrent.ConcurrentDictionary<int, string> registeredTypeNames = new();
    private static readonly Lock registerLock = new();
    private static bool defaultsRegistered = false;

    // Ids of the built-in types, in the order RegisterDefaults registers them.
    public const int AnyType       = 0;
    public const int IntegerType   = 1;
    public const int FloatType     = 2;
    public const int BooleanType   = 3;
    public const int DateType      = 4;
    public const int HashmapType   = 5;
    public const int GeneratorType = 6;
    public const int LambdaType    = 7;
    public const int StringType    = 8;
    public const int ListType      = 9;
    public const int BytesType     = 10;
    public const int ObjectType    = 11;
    public const int PointerType   = 12;
    public const int NullType      = 13;

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
        RegisterType("generator");
        RegisterType("lambda");
        RegisterType("string");
        RegisterType("list");
        RegisterType("bytes");
        RegisterType("object");
        RegisterType("pointer");
        RegisterType("null");

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
            Typing.ValueType.None => "null",
            Typing.ValueType.String => "string",
            Typing.ValueType.Pointer => "pointer",
            Typing.ValueType.Bytes => "bytes",
            Typing.ValueType.Generator => "generator",
            Typing.ValueType.Package => "package",
            Typing.ValueType.Struct => "struct",
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
        // Ids come from the count, so check-and-assign must be one step.
        lock (registerLock)
        {
            if (registeredTypes.ContainsKey(typeName))
                return;
            var type = registeredTypes.Count;
            registeredTypeNames[type] = typeName;
            registeredTypes[typeName] = type;
        }
    }
}