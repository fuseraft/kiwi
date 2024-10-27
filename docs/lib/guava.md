# `guava`

The `guava` package contains functionality for registering and running tests with assertions.

## Table of Contents

- [Package Functions](#package-functions)
  - [`initialize()`](#initialize)
  - [`assert(condition, msg)`](#assertcondition-msg--assertion-failed)
  - [`register_test(name, t)`](#register_testname-t)
  - [`run_test(name, test, results)`](#run_testname-test-results--)
  - [`run_tests()`](#run_tests)

## Package Functions

### `initialize()`
Initializes the global testing environment if it hasn't been initialized yet.

**Parameters**
_None_

**Returns**
_None_

---

### `assert(condition, msg = "Assertion failed.")`
Checks if a condition is true. If the condition is `false`, an error is thrown with the specified message.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Boolean` | `condition` | The condition to evaluate. |
| `String` | `msg` | The message to display if the assertion fails (optional, defaults to `"Assertion failed."`). |

**Returns**
_None_

---

### `register_test(name, t)`
Registers a test function to the global test list.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `name` | The name of the test. |
| `Function` | `t` | The test function. |

**Returns**
_None_

---

### `run_test(name, test, results = [])`
Runs a specific test, measures its duration, and adds the result to the `results` list.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `name` | The name of the test. |
| `Function` | `test` | The test function to execute. |
| `List` | `results` | The list where test results are stored (optional, defaults to an empty list). |

**Returns**
_None_

---

### `run_tests()`
Executes all registered tests and returns a list of test results, including the name, whether the test passed, and its duration.

**Parameters**
_None_

**Returns**
| Type | Description |
| :--- | :--- |
| `List` | A list of test result objects, each containing the `name`, `result` (boolean), and `duration` (milliseconds). |
