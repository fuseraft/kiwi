---
title: native
description: Load native extensions written in C, Rust, Zig or C#.
---

The `native` package loads native extensions: shared libraries (`.so`, `.dylib`, `.dll`) that add functions to Kiwi. Use one when you need a C library, an operating system API, or code that runs faster than Kiwi can.

```kiwi nocheck
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

`native::load` is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode). An extension runs with the same permissions as the interpreter, so only load libraries you trust.

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

Any language that can export a C function works. That includes C#: a class library published with native AOT and `<NativeLib>Shared</NativeLib>` exports `kiwi_ext_init` through `[UnmanagedCallersOnly(EntryPoint = "kiwi_ext_init")]`.

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

```kiwi nocheck
try
  mymath::divide(1, 0)
catch (kind, message)
  println "${kind}: ${message}"   # NativeError: division by zero
end
```

A function defined with a fixed arity throws a `ParameterCountMismatchError` when called with the wrong number of arguments, before the native code runs.

<!-- api: native -->
