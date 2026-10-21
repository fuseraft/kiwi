---
title: tester
description: A small unit-testing framework.
---

The `tester` package registers and runs unit tests. [Testing](/kiwi/tools/testing/) walks through it with examples; this page is the package reference.

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

<!-- api: tester -->
