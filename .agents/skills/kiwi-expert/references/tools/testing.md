# Testing

> Writing and running unit tests with the tester package.

The [`tester`](../lib/tester.md) package is a small unit-testing framework. Register tests with the `@tester::test` decorator, check results with assertions, and run them with `tester::run_tests()`.

## Writing tests

```kiwi
fn slugify(text)
  text.lowercase().trim().replace(" ", "-")
end

@tester::test("slugify lowercases and joins words")
fn test_slugify()
  tester::assert_eq(slugify("Hello World"), "hello-world", "basic slug")
end

@tester::test("slugify trims whitespace")
fn test_trim()
  tester::assert_eq(slugify("  Kiwi "), "kiwi")
end

@tester::test("lists grow when pushed")
fn test_push()
  items = [1, 2]
  items.push(3)
  tester::assert(items.size() == 3, "expected three items")
end

results = tester::run_tests()
```

Each test prints a line as it runs:

```text
Running test: slugify lowercases and joins words       PASSED in 0.06ms
Running test: slugify trims whitespace                 PASSED in 0.01ms
Running test: lists grow when pushed                   PASSED in 0.01ms
```

A failing assertion stops that test, prints the message, and moves on to the next test.

## Assertions

| Function | Fails when |
| :-- | :-- |
| `tester::assert(condition, message)` | `condition` is false |
| `tester::assert_eq(actual, expected, message)` | `actual` and `expected` serialize differently |

`assert_eq` compares the serialized form of both values, so lists and hashmaps are compared by content. Any error thrown inside a test also fails it, so you can use your own checks or [`error::assert`](../lib/error.md).

## Results

`run_tests()` returns a list with one hashmap per test: `name`, `result` (`true` if it passed) and `duration` in milliseconds. Use it to set the exit code, which is what CI looks at:

```kiwi
@tester::test("math works")
fn test_math()
  tester::assert_eq(2 ** 10, 1024)
end

results = tester::run_tests()
failed = results.filter(do (r) => !r["result"])

println "${results.size() - failed.size()}/${results.size()} passed"
exit 1 when failed.size() > 0
```

## Testing Kiwi itself

The interpreter's own test suite is written in Kiwi. From a clone of the repository:

```sh
./build.sh
./bin/kiwi tests/test           # everything
./bin/kiwi tests/test_stdlib    # standard library only
```

Language tests live in `tests/lib/suite/` and standard library tests in `tests/lib/stdlib/`. The runner discovers them automatically and exits with `1` if anything fails.
