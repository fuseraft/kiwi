# `tester`

The `tester` package contains functionality for registering and running tests with assertions.

---

## Package Functions

### `initialize()`
Initializes the global testing environment if it hasn't been initialized yet.

**Parameters**
_None_

**Returns**
_None_

**Example**

```kiwi
import "tester"

tester::initialize()
```

---

### `assert(condition, msg = "Assertion failed.")`
Checks if a condition is true. If the condition is `false`, an error is thrown with the specified message.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `boolean` | `condition` | The condition to evaluate. |
| `string` | `msg` | The message to display if the assertion fails (optional, defaults to `"Assertion failed."`). |

**Returns**
_None_

**Example**

```kiwi
import "tester"

tester::assert(1 + 1 == 2, "basic math should work")
tester::assert("hello".size() == 5, "string size should be 5")
```

---

### `assert_eq(actual, expected, msg = "")`
Checks if `actual` equals `expected`. If they are not equal, an error is thrown with an informative message.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `actual` | The actual value. |
| `any` | `expected` | The expected value. |
| `string` | `msg` | An optional message to include in the failure output. |

**Returns**
_None_

**Example**

```kiwi
import "tester"

var result = [1, 2, 3].size()
tester::assert_eq(result, 3, "list should have 3 elements")

var greeting = "hello " + "world"
tester::assert_eq(greeting, "hello world", "string concatenation")
```

---

### `register_test(name, t)`
Registers a test function to the global test list.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `name` | The name of the test. |
| `lambda` | `t` | The test function. |

**Returns**
_None_

**Example**

```kiwi
import "tester"

tester::initialize()

tester::register_test("math works", with () do
  tester::assert_eq(2 + 2, 4, "addition")
  tester::assert_eq(10 - 3, 7, "subtraction")
end)

tester::register_test("strings work", with () do
  tester::assert_eq("hi".size(), 2, "string size")
  tester::assert("hello".contains("ell"), "substring check")
end)
```

---

### `run_test(name, test, results = [])`
Runs a specific test, measures its duration, and adds the result to the `results` list.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `name` | The name of the test. |
| `lambda` | `test` | The test function to execute. |
| `list` | `results` | The list where test results are stored (optional, defaults to an empty list). |

**Returns**
_None_

**Example**

```kiwi
import "tester"

var results = []

tester::run_test("addition", with () do
  tester::assert_eq(1 + 1, 2, "1 + 1 should equal 2")
end, results)

tester::run_test("string length", with () do
  tester::assert_eq("kiwi".size(), 4, "kiwi has 4 characters")
end, results)

for r in results
  println r["name"] + ": " + (r["result"] ? "passed" : "failed")
end
```

---

### `run_tests()`
Executes all registered tests and returns a list of test results, including the name, whether the test passed, and its duration.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of test result objects, each containing the `name`, `result` (boolean), and `duration` (milliseconds). |

**Example**

```kiwi
import "tester"

tester::initialize()

tester::register_test("list operations", with () do
  var nums = [1, 2, 3, 4, 5]
  tester::assert_eq(nums.size(), 5, "list size")
  tester::assert_eq(nums.sum(), 15, "list sum")
  tester::assert_eq(nums.first(), 1, "first element")
  tester::assert_eq(nums.last(), 5, "last element")
end)

tester::register_test("string operations", with () do
  var s = "Hello, Kiwi!"
  tester::assert(s.contains("Kiwi"), "should contain 'Kiwi'")
  tester::assert_eq(s.upcase(), "HELLO, KIWI!", "upcase")
  tester::assert_eq(s.replace("Kiwi", "World"), "Hello, World!", "replace")
end)

tester::register_test("arithmetic", with () do
  tester::assert_eq(2 ** 8, 256, "exponentiation")
  tester::assert_eq(17 % 5, 2, "modulo")
  tester::assert(3.14 > 3, "float comparison")
end)

var results = tester::run_tests()

for r in results
  var status = r["result"] ? "passed" : "failed"
  println r["name"] + " [" + status + "] (" + r["duration"].to_string() + "ms)"
end
```
