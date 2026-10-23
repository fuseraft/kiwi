---
title: ffi
description: Call C functions in shared libraries, and read and write native memory.
---

The `ffi` package calls functions in C libraries directly, without writing a [native extension](/kiwi/stdlib/native/). You describe a function's parameter and return types, and `ffi` converts Kiwi values to C and back:

```kiwi nocheck
libm = ffi::open("libm.so.6")
pow = ffi::func(libm, "pow", ["double", "double"], "double")
println pow(2, 10)   # 1024

libc = ffi::open("libc.so.6")
strlen = ffi::func(libc, "strlen", ["string"], "size_t")
println strlen("kiwi")   # 4
```

`ffi::open` takes a path or a library name and searches the way the platform does. A relative path is looked up next to the running script first. Library names differ between systems:

| Library | Linux | macOS | Windows |
| :-- | :-- | :-- | :-- |
| C standard library | `libc.so.6` | `libSystem.B.dylib` | `ucrtbase.dll` |
| Math | `libm.so.6` | `libSystem.B.dylib` | `ucrtbase.dll` |
| Your own `mylib` | `libmylib.so` | `libmylib.dylib` | `mylib.dll` |

`native::library_name("mylib")` gives the file name for the current platform.

`ffi` is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode). A C function runs with the interpreter's permissions, and a wrong type or a bad pointer can crash the interpreter instead of throwing an error. Check function signatures against the library's header.

## Types

| Type | C type | Kiwi value |
| :-- | :-- | :-- |
| `int8`, `int16`, `int32`, `int64` | `int8_t` … `int64_t` | `integer` |
| `uint8`, `uint16`, `uint32`, `uint64` | `uint8_t` … `uint64_t` | `integer` |
| `float`, `double` | `float`, `double` | `float`. An argument accepts an integer too. |
| `bool` | `bool` (C99) | `boolean` |
| `pointer` | any pointer | `pointer`, or `null` for a null pointer |
| `string` | `const char*` | `string`, or `null`. Arguments are copied as UTF-8 for the call; results are decoded, and a null result is `null`. |
| `bytes` | `const uint8_t*` | `bytes`, copied for the call (arguments only) |
| `void` | `void` | `null` (return type only) |

There are aliases for C's names:
- `char`, `uchar`, `short`, `ushort`, `int` and `uint` are the 8-, 16- and 32-bit types.
- `longlong`, `ulonglong`, `ssize_t` and `size_t` are 64-bit.
- `long` and `ulong` are 64-bit, except on Windows, where they are 32-bit as in C.

Kiwi integers are signed 64-bit, so a `uint64` result above `9223372036854775807` reads as a negative number. An argument outside its type's range is an error, not a silent truncation.

## Calling functions

`ffi::func(library, name, params, returns)` returns a lambda. Call it like any other:

```kiwi nocheck
libc = ffi::open("libc.so.6")
getenv = ffi::func(libc, "getenv", ["string"], "string")
println getenv("HOME")          # /home/kiwi
println getenv("NOT_SET")       # null

abs = ffi::func(libc, "labs", ["long"], "long")
println abs(-9000000000)        # 9000000000
```

`ffi::wrap(pointer, params, returns)` does the same for a function pointer, such as one a C function returns. `ffi::symbol(library, name)` gives the address of any exported symbol, including variables.

Errors are `FfiError`s:
- the wrong number of arguments: `add expects 2 arguments, got 1.`
- a value of the wrong type or out of range: `add: argument 1: 2147483648 is out of range for int32.`
- an unknown type, a missing symbol, or a library that won't load

### Limits

`ffi` works on Linux, macOS and Windows on x64 and arm64, without libffi or any other dependency. It passes arguments the way each platform's C calling convention does, which sets these limits:
- **Structs by value:** a function that takes or returns a struct by value can't be called. Most C APIs pass structs by pointer, which works; see below.
- **Variadic functions:** functions such as `printf` aren't supported.
- **Argument count:** up to 12 arguments on Windows, and more elsewhere: up to 6 integer and 8 floating-point arguments in registers on x64 Linux and macOS (8 and 8 on arm64), plus 8 more on the stack. On macOS arm64, only 8-byte types (`int64`, `uint64`, `double`, `pointer`, `string`) can go on the stack.
- **Callbacks:** C code can't call Kiwi lambdas. For that, write a [native extension](/kiwi/stdlib/native/).

## Memory

C functions often fill in memory you give them. `ffi::alloc(size)` allocates zeroed memory, and `ffi::read` and `ffi::write` access it by byte offset and type:

```kiwi
buf = ffi::alloc(4 * ffi::sizeof("int32"))
for i in [0 to 3] do
  ffi::write(buf, i * 4, "int32", i * i)
end
println [0 to 3].map(with (i) do return ffi::read(buf, i * 4, "int32") end)   # [0, 1, 4, 9]
ffi::free(buf)
```

A struct is memory with fields at fixed offsets. For `struct { int32_t x; double y; }`, `y` is at offset 8, because C aligns a `double` to 8 bytes:

```kiwi
point = ffi::alloc(16)
ffi::write(point, 0, "int32", 3)
ffi::write(point, 8, "double", 0.5)
println [ffi::read(point, 0, "int32"), ffi::read(point, 8, "double")]   # [3, 0.5]
ffi::free(point)
```

Strings and bytes:

```kiwi
s = ffi::cstring("héllo")        # a NUL-terminated UTF-8 copy
println ffi::string(s)           # héllo
println ffi::string(s, 1)        # h
println ffi::bytes(s, 3)         # the first 3 bytes
ffi::free(s)
```

Reading a `string` at an offset follows the `char*` stored there, which is how you read a `const char**` array or a string field in a struct.

Memory from `ffi::alloc` and `ffi::cstring` stays allocated until `ffi::free`. Freeing other memory, or freeing twice, crashes the interpreter. Memory a C function returns usually has to be freed by a function from the same library, as its documentation says.

`ffi::offset(p, n)` gives the pointer `n` bytes past `p`. `ffi::address(p)` and `ffi::pointer(address)` convert between pointers and integers, and `ffi::is_null(p)` checks for `null` or address 0.

<!-- api: ffi -->
