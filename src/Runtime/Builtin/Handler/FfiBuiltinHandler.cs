using System.Runtime.InteropServices;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Ffi;
using kiwi.Settings;
using kiwi.Tracing.Error;
using kiwi.Typing;
using kiwi.VM;

namespace kiwi.Runtime.Builtin.Handler;

/// <summary>
/// The <c>ffi</c> package: calling C functions in shared libraries, and reading and writing
/// native memory. None of it is available in safe mode.
/// </summary>
public static unsafe class FfiBuiltinHandler
{
    private const string ErrorType = "FfiError";

    // Libraries opened with ffi::open, so a stray pointer can't be passed to the loader. Opening
    // a library again returns the same handle, so each is counted until it is closed as often.
    private static readonly Dictionary<nint, (string Path, int Count)> _libraries = [];
    private static readonly object _librariesLock = new();

    public static Value Execute(KiwiVM vm, Token token, TokenName builtin, List<Value> args)
    {
        if (Kiwi.Settings.SafeMode) throw new SafeModeError(token, "ffi");

        try
        {
            return builtin switch
            {
                TokenName.Builtin_Ffi_Open       => Open(vm, token, args),
                TokenName.Builtin_Ffi_Close      => Close(token, args),
                TokenName.Builtin_Ffi_Symbol     => Symbol(token, args),
                TokenName.Builtin_Ffi_Signature  => Signature(token, args),
                TokenName.Builtin_Ffi_Call       => Call(token, args),
                TokenName.Builtin_Ffi_Alloc      => Alloc(token, args),
                TokenName.Builtin_Ffi_Free       => Free(token, args),
                TokenName.Builtin_Ffi_Read       => Read(token, args),
                TokenName.Builtin_Ffi_Write      => Write(token, args),
                TokenName.Builtin_Ffi_String     => String(token, args),
                TokenName.Builtin_Ffi_Bytes      => Bytes(token, args),
                TokenName.Builtin_Ffi_WriteBytes => WriteBytes(token, args),
                TokenName.Builtin_Ffi_CString    => CString(token, args),
                TokenName.Builtin_Ffi_SizeOf     => SizeOf(token, args),
                TokenName.Builtin_Ffi_Offset     => Offset(token, args),
                TokenName.Builtin_Ffi_Address    => Address(token, args),
                TokenName.Builtin_Ffi_Pointer    => Pointer(token, args),
                _ => throw new FunctionUndefinedError(token, token.Text),
            };
        }
        catch (FfiException e)
        {
            throw new KiwiError(token, ErrorType, e.Message);
        }
    }

    // -- Libraries and functions

    private static Value Open(KiwiVM vm, Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Open, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, FfiBuiltin.Open, 0, args[0]);

        var path = NativeBuiltinHandler.ResolvePath(vm, args[0].GetString());
        if (!NativeLibrary.TryLoad(path, out var library))
            throw new KiwiError(token, ErrorType, $"Could not load library `{path}`.");

        lock (_librariesLock)
            _libraries[library] = (path, _libraries.TryGetValue(library, out var open) ? open.Count + 1 : 1);
        return Value.CreatePointer(library);
    }

    private static Value Close(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Close, 1, args.Count);
        var library = Library(token, args[0]);
        lock (_librariesLock)
        {
            var open = _libraries[library];
            if (open.Count == 1) _libraries.Remove(library);
            else _libraries[library] = (open.Path, open.Count - 1);
        }
        NativeLibrary.Free(library);
        return Value.CreateNull();
    }

    private static Value Symbol(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Symbol, 2, args.Count);
        var library = Library(token, args[0]);
        ParameterTypeMismatchError.ExpectString(token, FfiBuiltin.Symbol, 1, args[1]);

        var name = args[1].GetString();
        if (!NativeLibrary.TryGetExport(library, name, out var address))
            throw new KiwiError(token, ErrorType, $"`{LibraryPath(library)}` has no symbol `{name}`.");
        return Value.CreatePointer(address);
    }

    /// <summary>Registers a signature: (name, parameter types, return type) → its number.</summary>
    private static Value Signature(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Signature, 3, args.Count);
        ParameterTypeMismatchError.ExpectString(token, FfiBuiltin.Signature, 0, args[0]);
        ParameterTypeMismatchError.ExpectList(token, FfiBuiltin.Signature, 1, args[1]);
        ParameterTypeMismatchError.ExpectString(token, FfiBuiltin.Signature, 2, args[2]);

        var name = args[0].GetString();
        var parameters = new List<FfiType>();
        foreach (var p in args[1].GetList())
        {
            var type = ParseType(token, p);
            if (type == FfiType.Void)
                throw new KiwiError(token, ErrorType, $"{name}: void isn't a parameter type.");
            parameters.Add(type);
        }

        var returns = ParseType(token, args[2]);
        if (returns == FfiType.Bytes)
            throw new KiwiError(token, ErrorType, $"{name}: bytes isn't a return type; return a pointer and read it with ffi::bytes.");

        return Value.CreateInteger(FfiSignature.Register(new FfiSignature(name, [.. parameters], returns)));
    }

    /// <summary>Calls a function pointer: (pointer, signature number, argument list).</summary>
    private static Value Call(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Call, 3, args.Count);
        var fn = FfiMarshal.PointerOf(args[0]);
        if (fn == 0)
            throw new KiwiError(token, ErrorType, "Can't call a null function pointer.");
        ParameterTypeMismatchError.ExpectInteger(token, FfiBuiltin.Call, 1, args[1]);
        ParameterTypeMismatchError.ExpectList(token, FfiBuiltin.Call, 2, args[2]);

        var signature = FfiSignature.Get(args[1].GetInteger())
            ?? throw new KiwiError(token, ErrorType, "Unknown ffi signature.");
        var values = args[2].GetList();
        var count = signature.Parameters.Length;
        if (values.Count != count)
            throw new KiwiError(token, ErrorType,
                $"{signature.Name} expects {count} argument{(count == 1 ? "" : "s")}, got {values.Count}.");

        var temporary = new List<nint>();
        try
        {
            var slots = new ulong[count];
            for (var i = 0; i < count; i++)
            {
                try
                {
                    slots[i] = FfiMarshal.ToSlot(values[i], signature.Parameters[i], temporary);
                }
                catch (FfiException e)
                {
                    throw new KiwiError(token, ErrorType, $"{signature.Name}: argument {i + 1}: {e.Message}");
                }
            }

            ulong result;
            try
            {
                result = FfiInvoker.Invoke(fn, slots, signature.Parameters, signature.Returns);
            }
            catch (InvalidOperationException e)
            {
                throw new KiwiError(token, ErrorType, $"{signature.Name}: {e.Message}");
            }

            return FfiMarshal.FromSlot(result, signature.Returns);
        }
        finally
        {
            foreach (var p in temporary)
                NativeMemory.Free((void*)p);
        }
    }

    // -- Memory

    private static Value Alloc(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Alloc, 1, args.Count);
        var size = Count(token, FfiBuiltin.Alloc, 0, args[0]);
        return Value.CreatePointer((nint)NativeMemory.AllocZeroed((nuint)Math.Max(size, 1)));
    }

    private static Value Free(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Free, 1, args.Count);
        NativeMemory.Free((void*)FfiMarshal.PointerOf(args[0]));
        return Value.CreateNull();
    }

    private static Value Read(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Read, 3, args.Count);
        var address = At(token, FfiBuiltin.Read, args[0], args[1]);
        return FfiMarshal.Read(address, ParseType(token, args[2]));
    }

    private static Value Write(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Write, 4, args.Count);
        var address = At(token, FfiBuiltin.Write, args[0], args[1]);
        FfiMarshal.Write(address, ParseType(token, args[2]), args[3]);
        return Value.CreateNull();
    }

    /// <summary>Decodes a UTF-8 C string: up to its NUL, or exactly the given number of bytes.</summary>
    private static Value String(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, FfiBuiltin.String, 1, 2, args.Count);
        var p = FfiMarshal.PointerOf(args[0]);
        if (p == 0) return Value.CreateNull();
        if (args.Count == 1 || args[1].IsNull())
            return Value.CreateString(Marshal.PtrToStringUTF8(p) ?? string.Empty);

        var length = Count(token, FfiBuiltin.String, 1, args[1]);
        return Value.CreateString(System.Text.Encoding.UTF8.GetString((byte*)p, (int)length));
    }

    private static Value Bytes(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Bytes, 2, args.Count);
        var p = NonNull(token, args[0]);
        var length = Count(token, FfiBuiltin.Bytes, 1, args[1]);
        return Value.CreateBytes(new ReadOnlySpan<byte>((void*)p, (int)length).ToArray());
    }

    private static Value WriteBytes(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.WriteBytes, 3, args.Count);
        var address = At(token, FfiBuiltin.WriteBytes, args[0], args[1]);
        ParameterTypeMismatchError.ExpectBytes(token, FfiBuiltin.WriteBytes, 2, args[2]);
        args[2].GetBytes().CopyTo(new Span<byte>((void*)address, args[2].GetBytes().Length));
        return Value.CreateNull();
    }

    /// <summary>A NUL-terminated UTF-8 copy of a string, freed with ffi::free.</summary>
    private static Value CString(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.CString, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, FfiBuiltin.CString, 0, args[0]);
        var data = System.Text.Encoding.UTF8.GetBytes(args[0].GetString());
        var p = (byte*)NativeMemory.Alloc((nuint)data.Length + 1);
        data.CopyTo(new Span<byte>(p, data.Length));
        p[data.Length] = 0;
        return Value.CreatePointer((nint)p);
    }

    private static Value SizeOf(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.SizeOf, 1, args.Count);
        return Value.CreateInteger(ParseType(token, args[0]).Size());
    }

    private static Value Offset(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Offset, 2, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, FfiBuiltin.Offset, 1, args[1]);
        return Value.CreatePointer(FfiMarshal.PointerOf(args[0]) + (nint)args[1].GetInteger());
    }

    private static Value Address(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Address, 1, args.Count);
        return Value.CreateInteger(FfiMarshal.PointerOf(args[0]));
    }

    private static Value Pointer(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, FfiBuiltin.Pointer, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, FfiBuiltin.Pointer, 0, args[0]);
        return Value.CreatePointer((nint)args[0].GetInteger());
    }

    // -- Helpers

    private static FfiType ParseType(Token token, Value value)
    {
        if (!value.IsString())
            throw new KiwiError(token, ErrorType, $"An ffi type is a string, such as \"int32\" or \"pointer\"; got {TypeRegistry.GetTypeName(value)}.");
        if (!FfiTypes.TryParse(value.GetString(), out var type))
            throw new KiwiError(token, ErrorType, $"Unknown ffi type `{value.GetString()}`.");
        return type;
    }

    private static nint Library(Token token, Value value)
    {
        var p = value.IsPointer() ? value.GetPointer() : 0;
        lock (_librariesLock)
        {
            if (p == 0 || !_libraries.ContainsKey(p))
                throw new KiwiError(token, ErrorType, "Expected a library opened with ffi::open.");
        }
        return p;
    }

    private static string LibraryPath(nint library)
    {
        lock (_librariesLock)
            return _libraries.TryGetValue(library, out var open) ? open.Path : "library";
    }

    private static nint NonNull(Token token, Value value)
    {
        var p = FfiMarshal.PointerOf(value);
        return p != 0 ? p : throw new KiwiError(token, ErrorType, "The pointer is null.");
    }

    // The address `offset` bytes past a non-null pointer.
    private static nint At(Token token, string function, Value pointer, Value offset)
    {
        var p = NonNull(token, pointer);
        ParameterTypeMismatchError.ExpectInteger(token, function, 1, offset);
        return p + (nint)offset.GetInteger();
    }

    private static long Count(Token token, string function, int position, Value value)
    {
        ParameterTypeMismatchError.ExpectInteger(token, function, position, value);
        var n = value.GetInteger();
        if (n < 0 || n > int.MaxValue)
            throw new KiwiError(token, ErrorType, $"{n} isn't a valid size.");
        return n;
    }
}
