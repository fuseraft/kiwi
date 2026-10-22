using System.Collections.Concurrent;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Tracing.Error;
using kiwi.Typing;
using kiwi.VM;

namespace kiwi.Runtime.Native;

/// <summary>
/// Loads native extensions and implements the <c>KiwiApi</c> table they call back into.
/// Everything here is native AOT safe: native code is reached through function pointers
/// with signatures fixed at compile time, and calls back through
/// <see cref="UnmanagedCallersOnlyAttribute"/> methods.
/// </summary>
public static unsafe class NativeHost
{
    public const string InitSymbol = "kiwi_ext_init";
    public const string ErrorType  = "NativeError";

    private static readonly KiwiHostApi* _api = CreateApi();
    private static readonly ConcurrentDictionary<string, NativeExtension> _loaded = new();
    private static readonly object _loadLock = new();

    /// <summary>
    /// Loads the library at <paramref name="path"/>, runs its <c>kiwi_ext_init</c>, and
    /// registers its functions as <c>packageName::name</c>. Loading a library twice
    /// returns the extension from the first load.
    /// </summary>
    public static NativeExtension Load(KiwiVM vm, Token token, string path, string packageName)
    {
        lock (_loadLock)
        {
            if (_loaded.TryGetValue(path, out var existing))
                return existing;

            if (!NativeLibrary.TryLoad(path, out var library))
                throw new KiwiError(token, ErrorType, $"Could not load native library `{path}`.");

            if (!NativeLibrary.TryGetExport(library, InitSymbol, out var init))
            {
                NativeLibrary.Free(library);
                throw new KiwiError(token, ErrorType, $"Native library `{path}` does not export `{InitSymbol}`.");
            }

            var ext = new NativeExtension(path, packageName, library);
            var ctx = NativeCallContext.Rent(token, vm, ext);
            try
            {
                var status = ((delegate* unmanaged[Cdecl]<KiwiHostApi*, nint, int>)init)(_api, ctx.Handle);
                if (ctx.Error != null)
                    throw AsKiwiError(ctx.Error, token);
                if (status != 0)
                    throw new KiwiError(token, ErrorType, $"`{InitSymbol}` in `{path}` failed with status {status}.");
            }
            catch
            {
                // Leave the library loaded: native code may still hold pointers into it.
                foreach (var name in ext.Functions)
                    vm.Context.Functions.TryRemove(name, out _);
                throw;
            }
            finally
            {
                ctx.Return();
            }

            _loaded[path] = ext;
            return ext;
        }
    }

    private static Value Invoke(nint fn, string name, int arity, IReadOnlyList<Value> args, Token token)
    {
        if (arity >= 0)
            ParameterCountMismatchError.Check(token, name, arity, args.Count);

        var ctx  = NativeCallContext.Rent(token, KiwiVM.Current);
        var argc = args.Count;
        Span<ulong> argv = argc <= 16 ? stackalloc ulong[argc] : new ulong[argc];
        try
        {
            for (var i = 0; i < argc; i++)
                argv[i] = ctx.Add(args[i]);

            ulong result;
            fixed (ulong* p = argv)
                result = ((delegate* unmanaged[Cdecl]<nint, int, ulong*, ulong>)fn)(ctx.Handle, argc, p);

            if (ctx.Error != null)
                throw AsKiwiError(ctx.Error, token);

            return result == 0 ? Value.CreateNull() : ctx.Get(result);
        }
        finally
        {
            ctx.Return();
        }
    }

    private static Exception AsKiwiError(Exception error, Token token) => error switch
    {
        KiwiError ke => ke,
        _            => new KiwiError(token, ErrorType, error.Message),
    };

    private static KiwiError TypeError(NativeCallContext ctx, string function, string expected, Value actual)
    {
        return new KiwiError(ctx.Token, ErrorType,
            $"{function}: expected {expected}, got {TypeRegistry.GetTypeName(actual)}.");
    }

    private static string ReadUtf8(byte* s)
    {
        return s == null ? string.Empty : Marshal.PtrToStringUTF8((nint)s) ?? string.Empty;
    }

    private static KiwiHostApi* CreateApi()
    {
        var api = (KiwiHostApi*)NativeMemory.AllocZeroed((nuint)sizeof(KiwiHostApi));
        api->AbiVersion  = KiwiHostApi.CurrentAbiVersion;
        api->Size        = (uint)sizeof(KiwiHostApi);
        api->Define      = &Define;
        api->Raise       = &Raise;
        api->TypeOf      = &TypeOf;
        api->NewNone     = &NewNone;
        api->NewInt      = &NewInt;
        api->NewFloat    = &NewFloat;
        api->NewBool     = &NewBool;
        api->NewString   = &NewString;
        api->NewBytes    = &NewBytes;
        api->NewPointer  = &NewPointer;
        api->ToInt       = &ToInt;
        api->ToFloat     = &ToFloat;
        api->ToBool      = &ToBool;
        api->ToPointer   = &ToPointer;
        api->StringGet   = &StringGet;
        api->BytesGet    = &BytesGet;
        api->NewList     = &NewList;
        api->ListLen     = &ListLen;
        api->ListGet     = &ListGet;
        api->ListPush    = &ListPush;
        api->NewHashmap  = &NewHashmap;
        api->HashmapGet  = &HashmapGet;
        api->HashmapSet  = &HashmapSet;
        api->HashmapKeys = &HashmapKeys;
        api->Call        = &Call;
        return api;
    }

    // -- KiwiApi: every entry point catches, records the error, and returns 0, because an
    //    exception must never unwind through native frames.

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static int Define(nint c, byte* name, nint fn, int arity)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var ext = ctx.Initializing
                ?? throw new KiwiError(ctx.Token, ErrorType, "define: only valid inside kiwi_ext_init.");
            var vm = ctx.VM ?? throw new InvalidOperationException("define: no active VM.");

            var functionName = ReadUtf8(name);
            if (functionName.Length == 0 || fn == 0)
                throw new KiwiError(ctx.Token, ErrorType, "define: a function needs a name and a pointer.");

            var qualifiedName = ext.PackageName + "::" + functionName;
            var kf = new KFunction(new FunctionNode { Name = qualifiedName })
            {
                Name              = qualifiedName,
                IsPackageFunction = true,
                NativeBody        = (args, token) => Invoke(fn, qualifiedName, arity, args, token),
            };

            if (!vm.Context.Functions.TryAdd(qualifiedName, kf))
                throw new KiwiError(ctx.Token, ErrorType, $"define: `{qualifiedName}` is already defined.");

            ext.Functions.Add(qualifiedName);
            return 0;
        }
        catch (Exception e) { ctx.Fail(e); return -1; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static void Raise(nint c, byte* message)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try { ctx.Fail(new KiwiError(ctx.Token, ErrorType, ReadUtf8(message))); }
        catch (Exception e) { ctx.Fail(e); }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static int TypeOf(nint c, ulong v)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            return ctx.Get(v).Type switch
            {
                Typing.ValueType.None    => 0,
                Typing.ValueType.Integer => 1,
                Typing.ValueType.Float   => 2,
                Typing.ValueType.Boolean => 3,
                Typing.ValueType.String  => 4,
                Typing.ValueType.List    => 5,
                Typing.ValueType.Hashmap => 6,
                Typing.ValueType.Bytes   => 7,
                Typing.ValueType.Pointer => 8,
                Typing.ValueType.Lambda  => 9,
                _                        => 10,
            };
        }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong NewNone(nint c)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try { return ctx.Add(Value.CreateNull()); }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong NewInt(nint c, long v)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try { return ctx.Add(Value.CreateInteger(v)); }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong NewFloat(nint c, double v)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try { return ctx.Add(Value.CreateFloat(v)); }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong NewBool(nint c, int v)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try { return ctx.Add(Value.CreateBoolean(v != 0)); }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong NewString(nint c, byte* utf8, nuint len)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try { return ctx.Add(Value.CreateString(Encoding.UTF8.GetString(utf8, checked((int)len)))); }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong NewBytes(nint c, byte* data, nuint len)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try { return ctx.Add(Value.CreateBytes(new ReadOnlySpan<byte>(data, checked((int)len)).ToArray())); }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong NewPointer(nint c, nint p)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try { return ctx.Add(Value.CreatePointer(p)); }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static long ToInt(nint c, ulong v)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(v);
            return value.Type switch
            {
                Typing.ValueType.Integer => value.GetInteger(),
                Typing.ValueType.Float   => (long)value.GetFloat(),
                Typing.ValueType.Boolean => value.GetBoolean() ? 1 : 0,
                _ => throw TypeError(ctx, "to_int", "a number", value),
            };
        }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static double ToFloat(nint c, ulong v)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(v);
            return value.Type switch
            {
                Typing.ValueType.Float   => value.GetFloat(),
                Typing.ValueType.Integer => value.GetInteger(),
                _ => throw TypeError(ctx, "to_float", "a number", value),
            };
        }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static int ToBool(nint c, ulong v)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(v);
            if (!value.IsBoolean()) throw TypeError(ctx, "to_bool", "a boolean", value);
            return value.GetBoolean() ? 1 : 0;
        }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static nint ToPointer(nint c, ulong v)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(v);
            if (!value.IsPointer()) throw TypeError(ctx, "to_pointer", "a pointer", value);
            return value.GetPointer();
        }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static byte* StringGet(nint c, ulong v, nuint* len)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(v);
            if (!value.IsString()) throw TypeError(ctx, "string_get", "a string", value);
            var bytes = Encoding.UTF8.GetBytes(value.GetString());
            if (len != null) *len = (nuint)bytes.Length;
            return ctx.Pin(bytes, nulTerminate: true);
        }
        catch (Exception e) { ctx.Fail(e); return null; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static byte* BytesGet(nint c, ulong v, nuint* len)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(v);
            if (!value.IsBytes()) throw TypeError(ctx, "bytes_get", "bytes", value);
            var bytes = value.GetBytes();
            if (len != null) *len = (nuint)bytes.Length;
            return ctx.Pin(bytes, nulTerminate: false);
        }
        catch (Exception e) { ctx.Fail(e); return null; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong NewList(nint c)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try { return ctx.Add(Value.CreateList()); }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static long ListLen(nint c, ulong list)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(list);
            if (!value.IsList()) throw TypeError(ctx, "list_len", "a list", value);
            return value.GetList().Count;
        }
        catch (Exception e) { ctx.Fail(e); return -1; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong ListGet(nint c, ulong list, long index)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(list);
            if (!value.IsList()) throw TypeError(ctx, "list_get", "a list", value);
            if (index < 0 || index > int.MaxValue || !value.GetList().TryGet((int)index, out var item))
                throw new KiwiError(ctx.Token, ErrorType, $"list_get: index {index} is out of range.");
            return ctx.Add(item);
        }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static int ListPush(nint c, ulong list, ulong item)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(list);
            if (!value.IsList()) throw TypeError(ctx, "list_push", "a list", value);
            value.GetList().Add(ctx.Get(item));
            return 0;
        }
        catch (Exception e) { ctx.Fail(e); return -1; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong NewHashmap(nint c)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try { return ctx.Add(Value.CreateHashmap()); }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong HashmapGet(nint c, ulong map, ulong key)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(map);
            if (!value.IsHashmap()) throw TypeError(ctx, "hashmap_get", "a hashmap", value);
            return value.GetHashmap().TryGetValue(ctx.Get(key), out var item) ? ctx.Add(item) : 0;
        }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static int HashmapSet(nint c, ulong map, ulong key, ulong item)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(map);
            if (!value.IsHashmap()) throw TypeError(ctx, "hashmap_set", "a hashmap", value);
            value.GetHashmap()[ctx.Get(key)] = ctx.Get(item);
            return 0;
        }
        catch (Exception e) { ctx.Fail(e); return -1; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong HashmapKeys(nint c, ulong map)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var value = ctx.Get(map);
            if (!value.IsHashmap()) throw TypeError(ctx, "hashmap_keys", "a hashmap", value);
            return ctx.Add(Value.CreateList(value.GetHashmap().Keys));
        }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }

    [UnmanagedCallersOnly(CallConvs = [typeof(CallConvCdecl)])]
    private static ulong Call(nint c, ulong callable, int argc, ulong* argv)
    {
        var ctx = NativeCallContext.FromHandle(c);
        try
        {
            var vm    = ctx.VM ?? throw new InvalidOperationException("call: no active VM.");
            var value = ctx.Get(callable);
            if (!vm.TryResolveCallable(value, out var target, out var name) || target == null)
                throw TypeError(ctx, "call", "a lambda or function", value);

            var args = new List<Value>(argc);
            for (var i = 0; i < argc; i++)
                args.Add(ctx.Get(argv[i]));

            return ctx.Add(vm.InvokeCallable(target, args, ctx.Token, name ?? "<native call>"));
        }
        catch (Exception e) { ctx.Fail(e); return 0; }
    }
}
