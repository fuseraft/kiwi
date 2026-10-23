# Kiwi.Native

Write [Kiwi](https://github.com/fuseraft/kiwi) native extensions in C#. Mark static methods with `[KiwiFunction]`, publish the project with native AOT, and load the library with `native::load`:

```csharp
using Kiwi.Native;

public static class MyMath
{
    [KiwiFunction]
    public static long Add(long a, long b) => a + b;

    [KiwiFunction]
    public static double Mean(params double[] xs) => xs.Length == 0 ? 0 : xs.Average();
}
```

```sh
dotnet publish -c Release -r linux-x64 -o out
```

```kiwi
native::load("out/mymath.so", "mymath")
println mymath::add(2, 3)        # 5
println mymath::mean(1, 2, 4.5)  # 2.5
```

A source generator writes the `kiwi_ext_init` export and a wrapper for each function. The wrapper converts the arguments and result, and reports exceptions as Kiwi errors. There's no C to write, and nothing uses reflection, so the library trims and compiles ahead of time.

## Setting up a project

Create a class library and reference the package:

```sh
dotnet new classlib -n mymath
cd mymath
dotnet add package Kiwi.Native
```

Then turn on native AOT in `mymath.csproj`. It has to be set in the project itself; the package can't set it for you:

```xml
<PropertyGroup>
  <PublishAot>true</PublishAot>
</PropertyGroup>
```

The package sets `NativeLib=Shared`, which builds a shared library instead of an executable, and `AllowUnsafeBlocks`, which the generated code needs. Without `PublishAot`, `dotnet publish` stops with error `KIWI100`.

Publish for each platform you support. NativeAOT can't cross-compile between operating systems, so build each one on its own OS, for example in a CI matrix:

| Runtime ID | Library |
| :-- | :-- |
| `linux-x64`, `linux-arm64` | `mymath.so` |
| `osx-x64`, `osx-arm64` | `mymath.dylib` |
| `win-x64` | `mymath.dll` |

The file is named after the assembly, without the `lib` prefix that `native::library_name` adds, so pass the extension's name to `native::load`.

Extensions need .NET 8 or later to build, and Kiwi 1.6.2 or later to load. Each extension carries its own copy of the .NET runtime, so a library starts at about 1.5 MB.

## Functions

A `[KiwiFunction]` must be `static`, and it and the types containing it must be `public` or `internal`. Its Kiwi name is the method name in snake_case, so `SumSquares` becomes `sum_squares`. To pick a name yourself, use `[KiwiFunction("name")]`.

Parameters and results can be any of these types:

| .NET | Kiwi |
| :-- | :-- |
| `long`, `int` | `integer`. An `int` parameter rejects values outside its range. A result can also be `short`, `byte`, `uint` and the like. |
| `double`, `float` | `float`. A parameter accepts an integer too. |
| `bool` | `boolean` |
| `string` | `string` |
| `byte[]` | `bytes` |
| `nint` | `pointer` |
| `T[]`, `List<T>`, `IList<T>`, `IEnumerable<T>`, `IReadOnlyList<T>` | `list` |
| `Dictionary<K, V>`, `IDictionary<K, V>`, `IReadOnlyDictionary<K, V>` | `hashmap` |
| `object` | Any value, converted as described for `KiwiContext.ToObject` below. |
| `KiwiValue` | Any value, as a raw handle. |
| `void` | `null` (results only). |

Collections nest, as in `Dictionary<string, List<long[]>>`. A result can be any type that implements `IEnumerable<T>`, and a sequence of `KeyValuePair<K, V>` becomes a hashmap.

A parameter accepts `null` only when its type can hold it: `long?`, `string?`, `List<long>?` and so on. Returning `null` gives Kiwi `null`. The project needs nullable annotations enabled (`<Nullable>enable</Nullable>`) for `string?` to differ from `string`.

Parameters can have default values, and a `params` array takes the rest of the arguments:

```csharp
[KiwiFunction]
public static string Greet(string? name = null, string greeting = "Hello") => $"{greeting}, {name ?? "stranger"}!";

[KiwiFunction]
public static string Join(string separator, params string[] parts) => string.Join(separator, parts);
```

A function with a fixed number of parameters gets Kiwi's usual `ParameterCountMismatchError` when it's called with the wrong number. A function with defaults or `params` checks the count itself and throws a `NativeError`.

A method the generator can't export is a compile error with an ID from `KIWI001` to `KIWI011`. For example, `KIWI001` means a parameter has a type from the table above that isn't supported, and `KIWI008` means two methods share a Kiwi name.

## Errors

Any exception a function throws becomes a `NativeError` in Kiwi, with the exception's message:

```csharp
[KiwiFunction]
public static long Divide(long a, long b) => b == 0 ? throw new KiwiException("division by zero") : a / b;
```

```kiwi
try
  mymath::divide(1, 0)
catch (kind, message)
  println "${kind}: ${message}"   # NativeError: division by zero
end
```

An argument of the wrong type is reported with the function, the argument and the problem: `add: argument 1 (a): expected an integer, got string.`

## KiwiContext and raw values

Make the first parameter a `KiwiContext` to work with values directly, for example to accept anything, or to call a Kiwi lambda:

```csharp
[KiwiFunction]
public static List<object?> MapEach(KiwiContext ctx, KiwiValue fn, List<object?> items) =>
    items.Select(item => ctx.Invoke(fn, item)).ToList();
```

`KiwiContext` has these members:
- **Reading:** `TypeOf`, `GetInt64`, `GetDouble`, `GetString`, `GetBytes`, `GetList`, `HashmapKeys`, `TryGetValue` and the like. Each throws a `KiwiException` when the value has another type.
- **Creating:** `Int`, `Float`, `String`, `NewList`, `Push`, `NewHashmap`, `Set` and the like.
- **Converting:** `ToObject` turns any value into plain .NET objects (`long`, `double`, `bool`, `string`, `byte[]`, `nint`, `List<object?>`, `Dictionary<object, object?>`). `From` converts the other way.
- **Calling Kiwi:** `Call` calls a lambda or function with raw values, and `Invoke` with plain objects.

A `KiwiValue`, and the context itself, are valid only until the function returns. Don't store them. If a lambda passed to `Call` throws, `Call` throws a `KiwiException`, and the Kiwi error is what the caller sees, even if your code catches the exception.

## Keeping state

To keep a .NET object between calls, hand it to Kiwi as a pointer with `KiwiHandle`. Kiwi can't free it, so provide a function that does:

```csharp
[KiwiFunction] public static nint CounterNew() => KiwiHandle.Alloc(new Counter());
[KiwiFunction] public static long CounterBump(nint counter) => KiwiHandle.Get<Counter>(counter).Bump();
[KiwiFunction] public static void CounterFree(nint counter) => KiwiHandle.Free(counter);
```

## Threads

Kiwi tasks can call an extension's functions from several threads at once, so shared state needs locking, as in any multithreaded .NET code.

## Building the SDK

The SDK lives in [`sdk/csharp`](https://github.com/fuseraft/kiwi/tree/main/sdk/csharp) in the Kiwi repository. To try changes to it, pack it and add the folder as a package source:

```sh
dotnet pack Kiwi.Native -c Release -o ./nupkgs
dotnet nuget add source "$(pwd)/nupkgs" --name kiwi-local
```

The generator's tests are in `Kiwi.Native.Tests` (`dotnet test Kiwi.Native.Tests`). End-to-end tests for an extension built with the SDK are in `tests/native/csharp_ext.kiwi`.

To release a version, set `<Version>` in `Kiwi.Native/Kiwi.Native.csproj` and push a tag named `Kiwi.Native-v<version>`, such as `Kiwi.Native-v0.1.0`. The NuGet workflow tests the SDK, packs it, and publishes it to nuget.org.
