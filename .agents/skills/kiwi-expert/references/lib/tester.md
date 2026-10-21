# tester

> A small unit-testing framework.

The `tester` package registers and runs unit tests. [Testing](../tools/testing.md) walks through it with examples; this page is the package reference.

```kiwi
@tester::test("addition works")
fn test_addition()
  tester::assert_eq(1 + 1, 2, "basic addition")
end

results = tester::run_tests()
println results[0]["result"]   # true
```

| Function | Does |
| :-- | :-- |
| `@tester::test(name)` | Registers the decorated function as a test. |
| `register_test(name, lambda)` | Registers a test without a decorator. |
| `assert(condition, message)` | Fails the test if `condition` is false. |
| `assert_eq(actual, expected, message)` | Fails the test if the values differ. |
| `run_tests()` | Runs every registered test, prints a line for each, and returns a list of `{name, result, duration}` hashmaps. |
| `run_test(name, lambda, results)` | Runs one test and appends its result. |
| `registered(name)` | Whether a test with that name exists. |

## Functions

### `tester::assert_eq`

```kiwi
tester::assert_eq(actual: any, expected: any, msg: string = "")
```

Asserts that two values are equal, throwing on mismatch.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `actual` | `any` |  | The value produced by the code under test. |
| `expected` | `any` |  | The expected value. |
| `msg` | `string` | `""` | An optional message to include on failure. |

**Throws** `string` — A failure message if `actual != expected`.

### `tester::assert`

```kiwi
tester::assert(condition: boolean, msg: string = "assertion failed")
```

Asserts that a condition is true, throwing on failure.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `condition` | `boolean` |  | The condition to check. |
| `msg` | `string` | `"assertion failed"` | The failure message (default `"assertion failed"`). |

**Throws** `string` — A failure message if `condition` is `false`.

### `tester::register_test`

```kiwi
tester::register_test(name: string, t: lambda)
```

Registers a named test lambda for later execution by `run_tests()`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The display name of the test. |
| `t` | `lambda` |  | A no-argument lambda containing the test body. |

### `tester::registered`

```kiwi
tester::registered(name: string): boolean
```

Returns true if a test with the given name has been registered.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The test name to look up. |

**Returns** `boolean` — `true` if the test is registered.

### `tester::test`

```kiwi
tester::test(f: lambda, name: string): lambda
```

Decorator that registers a function as a named unit test.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `f` | `lambda` |  | The test function (injected by the decorator machinery). |
| `name` | `string` |  | The display name shown in the test runner output. |

**Returns** `lambda` — The original function, unchanged.

```kiwi
@tester::test("addition works")
fn test_addition()
  tester::assert(1 + 1 == 2, "basic addition")
end
```

### `tester::run_test`

```kiwi
tester::run_test(name: string, test: lambda, results: list = [])
```

Runs a single named test and prints the result.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The display name of the test. |
| `test` | `lambda` |  | The test lambda to execute. |
| `results` | `list` | `[]` | A list to append the result hashmap to. |

### `tester::run_tests`

```kiwi
tester::run_tests()
```

Runs all registered tests and returns a summary list.

**Returns** `list` — A list of result hashmaps, each with `name`, `result`, and `duration` keys.
