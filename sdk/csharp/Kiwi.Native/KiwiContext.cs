using System.Collections;
using System.Text;

namespace Kiwi.Native;

/// <summary>
/// Reads and creates Kiwi values during one native call. Take it as the first parameter of a
/// <see cref="KiwiFunctionAttribute"/> method to work with <see cref="KiwiValue"/> handles
/// directly, or to call a Kiwi lambda. Like the handles it creates, it is valid only until the
/// call returns.
/// </summary>
public readonly unsafe struct KiwiContext
{
    private readonly nint _ctx;

    internal KiwiContext(nint ctx) => _ctx = ctx;

    private static KiwiApi* Api => KiwiRuntime.Api;

    // -- Inspection

    public KiwiType TypeOf(KiwiValue value) =>
        value.IsEmpty ? KiwiType.None : (KiwiType)Api->TypeOf(_ctx, value.Handle);

    public bool IsNone(KiwiValue value) => TypeOf(value) == KiwiType.None;

    // -- Reading. Each throws a KiwiException when the value has another type.

    public long GetInt64(KiwiValue value)
    {
        Expect(value, KiwiType.Integer);
        return Api->ToInt(_ctx, value.Handle);
    }

    public int GetInt32(KiwiValue value)
    {
        var n = GetInt64(value);
        if (n is < int.MinValue or > int.MaxValue)
            throw new KiwiException($"{n} is out of range for a 32-bit integer.");
        return (int)n;
    }

    /// <summary>Reads a float, or an integer as a float.</summary>
    public double GetDouble(KiwiValue value)
    {
        var type = TypeOf(value);
        if (type is not (KiwiType.Float or KiwiType.Integer))
            throw TypeMismatch("a number", type);
        return Api->ToFloat(_ctx, value.Handle);
    }

    public float GetSingle(KiwiValue value) => (float)GetDouble(value);

    public bool GetBoolean(KiwiValue value)
    {
        Expect(value, KiwiType.Boolean);
        return Api->ToBool(_ctx, value.Handle) != 0;
    }

    public string GetString(KiwiValue value)
    {
        Expect(value, KiwiType.String);
        nuint len;
        var data = Api->StringGet(_ctx, value.Handle, &len);
        return data == null ? string.Empty : Encoding.UTF8.GetString(data, checked((int)len));
    }

    public byte[] GetBytes(KiwiValue value) => GetBytesSpan(value).ToArray();

    /// <summary>The bytes without copying them. The span is valid only until the call returns.</summary>
    public ReadOnlySpan<byte> GetBytesSpan(KiwiValue value)
    {
        Expect(value, KiwiType.Bytes);
        nuint len;
        var data = Api->BytesGet(_ctx, value.Handle, &len);
        return data == null ? [] : new ReadOnlySpan<byte>(data, checked((int)len));
    }

    public nint GetPointer(KiwiValue value)
    {
        Expect(value, KiwiType.Pointer);
        return Api->ToPointer(_ctx, value.Handle);
    }

    // -- Lists

    public int ListCount(KiwiValue list)
    {
        Expect(list, KiwiType.List);
        return checked((int)Api->ListLen(_ctx, list.Handle));
    }

    public KiwiValue ListGet(KiwiValue list, int index) =>
        Check(Api->ListGet(_ctx, list.Handle, index), "list_get");

    /// <summary>The list's items as handles.</summary>
    public KiwiValue[] GetList(KiwiValue list)
    {
        var items = new KiwiValue[ListCount(list)];
        for (var i = 0; i < items.Length; i++)
            items[i] = ListGet(list, i);
        return items;
    }

    public T[] GetArray<T>(KiwiValue list, Func<KiwiContext, KiwiValue, T> read)
    {
        var items = new T[ListCount(list)];
        for (var i = 0; i < items.Length; i++)
            items[i] = ReadItem(read, ListGet(list, i), i);
        return items;
    }

    public List<T> GetList<T>(KiwiValue list, Func<KiwiContext, KiwiValue, T> read) => [.. GetArray(list, read)];

    public KiwiValue NewList()
    {
        return Check(Api->NewList(_ctx), "new_list");
    }

    public KiwiValue NewList(IEnumerable<KiwiValue> items)
    {
        var list = NewList();
        foreach (var item in items)
            Push(list, item);
        return list;
    }

    public KiwiValue NewList<T>(IEnumerable<T>? items, Func<KiwiContext, T, KiwiValue> create)
    {
        if (items is null) return None();
        var list = NewList();
        foreach (var item in items)
            Push(list, create(this, item));
        return list;
    }

    public void Push(KiwiValue list, KiwiValue item)
    {
        if (Api->ListPush(_ctx, list.Handle, item.IsEmpty ? None().Handle : item.Handle) != 0)
            throw new KiwiException("list_push failed.");
    }

    // -- Hashmaps

    /// <summary>The hashmap's keys as handles, in order.</summary>
    public KiwiValue[] HashmapKeys(KiwiValue map)
    {
        Expect(map, KiwiType.Hashmap);
        return GetList(Check(Api->HashmapKeys(_ctx, map.Handle), "hashmap_keys"));
    }

    public bool TryGetValue(KiwiValue map, KiwiValue key, out KiwiValue value)
    {
        Expect(map, KiwiType.Hashmap);
        value = new KiwiValue(Api->HashmapGet(_ctx, map.Handle, key.Handle));
        return !value.IsEmpty;
    }

    public KiwiValue NewHashmap() => Check(Api->NewHashmap(_ctx), "new_hashmap");

    public void Set(KiwiValue map, KiwiValue key, KiwiValue value)
    {
        if (Api->HashmapSet(_ctx, map.Handle, key.Handle, value.IsEmpty ? None().Handle : value.Handle) != 0)
            throw new KiwiException("hashmap_set failed.");
    }

    public Dictionary<TKey, TValue> GetDictionary<TKey, TValue>(
        KiwiValue map,
        Func<KiwiContext, KiwiValue, TKey> readKey,
        Func<KiwiContext, KiwiValue, TValue> readValue) where TKey : notnull
    {
        var result = new Dictionary<TKey, TValue>();
        foreach (var key in HashmapKeys(map))
        {
            TryGetValue(map, key, out var value);
            result[ReadKey(readKey, key)] = ReadItem(readValue, value, key);
        }
        return result;
    }

    public KiwiValue NewHashmap<TKey, TValue>(
        IEnumerable<KeyValuePair<TKey, TValue>>? entries,
        Func<KiwiContext, TKey, KiwiValue> createKey,
        Func<KiwiContext, TValue, KiwiValue> createValue)
    {
        if (entries is null) return None();
        var map = NewHashmap();
        foreach (var (key, value) in entries)
            Set(map, createKey(this, key), createValue(this, value));
        return map;
    }

    // -- Creating

    public KiwiValue None() => Check(Api->NewNone(_ctx), "new_none");
    public KiwiValue Int(long value) => Check(Api->NewInt(_ctx, value), "new_int");
    public KiwiValue Float(double value) => Check(Api->NewFloat(_ctx, value), "new_float");
    public KiwiValue Bool(bool value) => Check(Api->NewBool(_ctx, value ? 1 : 0), "new_bool");
    public KiwiValue Pointer(nint value) => Check(Api->NewPointer(_ctx, value), "new_pointer");

    /// <summary>A Kiwi string, or <c>null</c> for a null string.</summary>
    public KiwiValue String(string? value)
    {
        if (value is null) return None();
        var bytes = Encoding.UTF8.GetBytes(value);
        fixed (byte* p = bytes)
            return Check(Api->NewString(_ctx, NotNull(p), (nuint)bytes.Length), "new_string");
    }

    /// <summary>Kiwi bytes, or <c>null</c> for a null array.</summary>
    public KiwiValue Bytes(byte[]? value) => value is null ? None() : Bytes(value.AsSpan());

    public KiwiValue Bytes(ReadOnlySpan<byte> value)
    {
        fixed (byte* p = value)
            return Check(Api->NewBytes(_ctx, NotNull(p), (nuint)value.Length), "new_bytes");
    }

    // `fixed` gives a null pointer for empty data, which Kiwi 1.7.0 rejects even with length 0.
    private static readonly byte* Empty = (byte*)System.Runtime.InteropServices.NativeMemory.Alloc(1);

    private static byte* NotNull(byte* p) => p == null ? Empty : p;

    // -- Plain .NET values

    /// <summary>
    /// Converts a .NET value to Kiwi: <c>null</c>, integers, floating-point numbers, booleans,
    /// strings and chars, <c>byte[]</c>, <c>nint</c> (a pointer), <see cref="KiwiValue"/>,
    /// dictionaries (a hashmap) and other sequences (a list), nested to any depth.
    /// </summary>
    public KiwiValue From(object? value) => value switch
    {
        null           => None(),
        KiwiValue v    => v.IsEmpty ? None() : v,
        bool b         => Bool(b),
        long n         => Int(n),
        int n          => Int(n),
        short n        => Int(n),
        sbyte n        => Int(n),
        byte n         => Int(n),
        ushort n       => Int(n),
        uint n         => Int(n),
        ulong n        => n <= long.MaxValue ? Int((long)n) : throw new KiwiException($"{n} is out of range for a Kiwi integer."),
        double d       => Float(d),
        float f        => Float(f),
        decimal m      => Float((double)m),
        string s       => String(s),
        char c         => String(c.ToString()),
        byte[] bytes   => Bytes(bytes),
        nint p         => Pointer(p),
        IDictionary d  => FromDictionary(d),
        IEnumerable e  => FromSequence(e),
        _ => throw new KiwiException($"A {value.GetType().Name} can't be converted to a Kiwi value."),
    };

    /// <summary>
    /// Converts a Kiwi value to .NET: <c>null</c>, <c>long</c>, <c>double</c>, <c>bool</c>,
    /// <c>string</c>, <c>byte[]</c>, <c>nint</c>, <c>List&lt;object?&gt;</c> or
    /// <c>Dictionary&lt;object, object?&gt;</c>. A lambda or other value stays a
    /// <see cref="KiwiValue"/>.
    /// </summary>
    public object? ToObject(KiwiValue value)
    {
        switch (TypeOf(value))
        {
            case KiwiType.None:    return null;
            case KiwiType.Integer: return GetInt64(value);
            case KiwiType.Float:   return GetDouble(value);
            case KiwiType.Boolean: return GetBoolean(value);
            case KiwiType.String:  return GetString(value);
            case KiwiType.Bytes:   return GetBytes(value);
            case KiwiType.Pointer: return GetPointer(value);
            case KiwiType.List:    return GetList(value, static (c, v) => c.ToObject(v));
            case KiwiType.Hashmap:
                var result = new Dictionary<object, object?>();
                foreach (var key in HashmapKeys(value))
                {
                    TryGetValue(value, key, out var item);
                    result[ToObject(key) ?? throw new KiwiException("A null hashmap key can't be converted to .NET.")] = ToObject(item);
                }
                return result;
            default:
                return value;
        }
    }

    // -- Calling Kiwi

    /// <summary>
    /// Calls a Kiwi lambda or function. If it throws, this throws too, and the Kiwi error is
    /// what the caller of the native function sees, even if C# code catches this exception.
    /// </summary>
    public KiwiValue Call(KiwiValue callable, params KiwiValue[] args)
    {
        var argv = new ulong[args.Length];
        for (var i = 0; i < args.Length; i++)
            argv[i] = args[i].IsEmpty ? None().Handle : args[i].Handle;

        ulong result;
        fixed (ulong* p = argv)
            result = Api->Call(_ctx, callable.Handle, argv.Length, p);
        return result == 0 ? throw new KiwiException("The Kiwi callback threw an error.") : new KiwiValue(result);
    }

    /// <summary><see cref="Call"/> with arguments and a result converted by <see cref="From"/> and <see cref="ToObject"/>.</summary>
    public object? Invoke(KiwiValue callable, params object?[] args)
    {
        var values = new KiwiValue[args.Length];
        for (var i = 0; i < args.Length; i++)
            values[i] = From(args[i]);
        return ToObject(Call(callable, values));
    }

    // -- Helpers

    private void Expect(KiwiValue value, KiwiType expected)
    {
        var actual = TypeOf(value);
        if (actual != expected)
            throw TypeMismatch(Describe(expected), actual);
    }

    private static KiwiException TypeMismatch(string expected, KiwiType actual) =>
        new($"expected {expected}, got {Name(actual)}.");

    private static KiwiValue Check(ulong handle, string operation) =>
        handle == 0 ? throw new KiwiException($"{operation} failed.") : new KiwiValue(handle);

    private T ReadItem<T>(Func<KiwiContext, KiwiValue, T> read, KiwiValue item, object position)
    {
        try { return read(this, item); }
        catch (KiwiException e) { throw new KiwiException($"item {Position(position)}: {e.Message}"); }
    }

    private T ReadKey<T>(Func<KiwiContext, KiwiValue, T> read, KiwiValue key)
    {
        try { return read(this, key); }
        catch (KiwiException e) { throw new KiwiException($"key: {e.Message}"); }
    }

    private string Position(object position) => position switch
    {
        int i       => i.ToString(),
        KiwiValue k => TypeOf(k) == KiwiType.String ? $"\"{GetString(k)}\"" : Name(TypeOf(k)),
        _           => position.ToString() ?? "",
    };

    private KiwiValue FromDictionary(IDictionary entries)
    {
        var map = NewHashmap();
        foreach (DictionaryEntry entry in entries)
            Set(map, From(entry.Key), From(entry.Value));
        return map;
    }

    private KiwiValue FromSequence(IEnumerable items)
    {
        var list = NewList();
        foreach (var item in items)
            Push(list, From(item));
        return list;
    }

    internal static string Name(KiwiType type) => type switch
    {
        KiwiType.None    => "null",
        KiwiType.Integer => "integer",
        KiwiType.Float   => "float",
        KiwiType.Boolean => "boolean",
        KiwiType.String  => "string",
        KiwiType.List    => "list",
        KiwiType.Hashmap => "hashmap",
        KiwiType.Bytes   => "bytes",
        KiwiType.Pointer => "pointer",
        KiwiType.Lambda  => "lambda",
        _                => "another type",
    };

    private static string Describe(KiwiType type) => type switch
    {
        KiwiType.Integer => "an integer",
        _                => "a " + Name(type),
    };
}
