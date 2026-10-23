# native

> Load native extensions written in C, C#, Rust or Zig.

The `native` package loads native extensions: shared libraries (`.so`, `.dylib`, `.dll`) that add functions to Kiwi. Use one when you need a C library, an operating system API, or code that runs faster than Kiwi can.

To call functions in an existing C library without writing an extension, use [`ffi`](ffi.md). An extension can do what `ffi` can't, such as passing structs by value or having C code call Kiwi lambdas.

```kiwi
m = native::load("libmymath.so")
println m.add(2, 3)         # 5
println mymath::add(2, 3)   # 5
```

`native::load(path, name)` loads the library and returns a reference to its package. The package is named `name`, or after the file when `name` is left out: `libmymath.so` becomes `mymath`. A relative `path` is looked up next to the running script first and then by the platform's library search. Loading a library a second time returns the same package.

Use `native::library_name` and `native::rid` to find the right binary on each platform:

```kiwi
println native::library_name("mymath")   # libmymath.so on Linux
println native::rid()                    # linux-x64, osx-arm64, win-x64, ...
```

`native::load` is unavailable in [safe mode](../tools/cli.md#safe-mode). An extension runs with the same permissions as the interpreter, so only load libraries you trust.

## Writing an extension

An extension is written against the C ABI in [`sdk/c/kiwi_ext.h`](https://github.com/fuseraft/kiwi/blob/main/sdk/c/kiwi_ext.h). It exports one function, `kiwi_ext_init`, which registers the extension's functions:

```c
#include "kiwi_ext.h"

static const KiwiApi* api;

static KiwiValue add(KiwiCtx* ctx, int argc, const KiwiValue* argv) {
    return api->new_int(ctx, api->to_int(ctx, argv[0]) + api->to_int(ctx, argv[1]));
}

KIWI_EXPORT int kiwi_ext_init(const KiwiApi* host, KiwiCtx* ctx) {
    if (host->abi_version < KIWI_ABI_VERSION) return 1;
    api = host;
    api->define(ctx, "add", add, 2);   // arity, or KIWI_VARIADIC
    return 0;
}
```

Build it as a shared library:

```sh
cc -shared -fPIC -I sdk/c mymath.c -o libmymath.so
```

Any language that can export a C function works, such as Rust or Zig. C# has an SDK that writes the C part for you.

### Writing an extension in C#

The [C# SDK](https://github.com/fuseraft/kiwi/tree/main/sdk/csharp) turns static methods marked `[KiwiFunction]` into Kiwi functions. A source generator writes `kiwi_ext_init` and the code that converts each function's arguments and result, and the library is published with native AOT:

```csharp
using Kiwi.Native;

public static class MyMath
{
    [KiwiFunction]
    public static long Add(long a, long b) => a + b;

    [KiwiFunction]
    public static double Mean(params double[] xs) => xs.Length == 0 ? 0 : xs.Average();

    [KiwiFunction]
    public static long Divide(long a, long b) => b == 0 ? throw new KiwiException("division by zero") : a / b;
}
```

The project references the `Kiwi.Native` package and sets `<PublishAot>true</PublishAot>`. Publish it for each platform; native AOT builds only for the operating system it runs on:

```sh
dotnet publish -c Release -r linux-x64 -o out
```

The library is named after the assembly, without a `lib` prefix, so give `native::load` the package name:

```kiwi
native::load("out/mymath.so", "mymath")
println mymath::add(2, 3)          # 5
println mymath::mean(1, 2, 4.5)    # 2.5
```

How the SDK maps types and arguments:
- **Types:** parameters and results can be `long`, `int`, `double`, `float`, `bool`, `string`, `byte[]` and `nint`. Collections can be arrays, lists and dictionaries of those types, nested to any depth. `object` accepts any value.
- **Nullable types:** a nullable type such as `string?` or `long?` accepts `null`.
- **Defaults and `params`:** default parameter values and `params` arrays work as they do in C#.
- **Names:** a method's Kiwi name is its name in snake_case, so `SumSquares` becomes `sum_squares`.
- **Errors:** an exception becomes a `NativeError` with the exception's message. An argument of the wrong type names the argument: `add: argument 1 (a): expected an integer, got string.`
- **Raw values:** to read raw values or call a Kiwi lambda, make the first parameter a `KiwiContext`.
- **Unsupported methods:** a method the generator can't export, such as one with an unsupported parameter type, is a compile error.

The SDK's [README](https://github.com/fuseraft/kiwi/blob/main/sdk/csharp/README.md) covers the full type table, `KiwiContext`, keeping .NET objects between calls with `KiwiHandle`, and building the package. Each extension carries its own copy of the .NET runtime, so a library starts at about 1.5 MB.

### Values and handles

Values cross the boundary as `KiwiValue` handles. Native code creates them with `new_int`, `new_string`, `new_list` and so on, and reads them with `type_of`, `to_int`, `string_get`, `list_get` and the rest of `KiwiApi`. A handle, and any pointer `string_get` or `bytes_get` returns, is valid only until the native function returns. Don't store them.

To keep native state between calls, return it as a pointer with `new_pointer` and read it back with `to_pointer`.

| Kiwi type | Create | Read |
| :-- | :-- | :-- |
| `null` | `new_none`, or return `0` | `type_of` is `KIWI_NONE` |
| `integer` | `new_int` | `to_int` |
| `float` | `new_float` | `to_float` |
| `boolean` | `new_bool` | `to_bool` |
| `string` | `new_string` (UTF-8) | `string_get` |
| `bytes` | `new_bytes` | `bytes_get` |
| `pointer` | `new_pointer` | `to_pointer` |
| `list` | `new_list`, `list_push` | `list_len`, `list_get` |
| `hashmap` | `new_hashmap`, `hashmap_set` | `hashmap_get`, `hashmap_keys` |

`api->call(ctx, fn, argc, argv)` calls a Kiwi lambda or function passed in as an argument.

### Errors

Call `api->raise(ctx, "message")` and return `0`. Once the function returns, Kiwi throws a `NativeError` with that message. Accessors that get the wrong type, such as `to_int` on a string, raise for you. If a lambda run by `api->call` throws, `call` returns `0`, and the error propagates once the native function returns.

```kiwi
try
  mymath::divide(1, 0)
catch (kind, message)
  println "${kind}: ${message}"   # NativeError: division by zero
end
```

A function defined with a fixed arity throws a `ParameterCountMismatchError` when called with the wrong number of arguments, before the native code runs.

## Functions

### `native::load`

```kiwi
native::load(path: string, name: string = "")
```

Loads a native extension and registers its functions as a package.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | Path to the shared library. A relative path is resolved against the running script's directory first. |
| `name` | `string` | `""` | The package name for the extension's functions. Defaults to the file name without its `lib` prefix and extension. |

**Returns** `package` — A reference to the extension's package.

**Throws** `NativeError` — When the library cannot be loaded, has no `kiwi_ext_init`, or its initialization fails.

```kiwi
m = native::load("libmymath.so")
println m.add(2, 3)
println mymath::add(2, 3)
```

### `native::rid`

```kiwi
native::rid(): string
```

Gets the runtime identifier of the running interpreter, used to pick a prebuilt native library.

**Returns** `string` — One of `linux-x64`, `linux-arm64`, `osx-x64`, `osx-arm64`, `win-x64`, and so on.

### `native::library_name`

```kiwi
native::library_name(base: string): string
```

Gets the platform's file name for a shared library.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `base` | `string` |  | The library's base name, such as `mymath`. |

**Returns** `string` — `libmymath.so` on Linux, `libmymath.dylib` on macOS, `mymath.dll` on Windows.
