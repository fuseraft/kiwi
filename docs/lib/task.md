# `task`

The `task` package contains functionality for concurrency in Kiwi.

## Table of Contents

- [Package Functions](#package-functions)
  - [`await(task_id)`](#awaittask_id)
  - [`busy()`](#busy)
  - [`spawn(callback, args)`](#spawncallback-args)
  - [`list()`](#list)
  - [`result(task_id)`](#resulttask_id)
  - [`sleep(ms)`](#sleepms)
  - [`status(task_id)`](#statustask_id)
  - [`wait()`](#wait)

## Package Functions

### `await(task_id)`
Await a specified task and return its result.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `task_id` | The task identifier. |

**Returns**
| Type | Description |
| :--- | :--- |
| `any` | The value returned by the task. |

---

### `busy()`
Returns `true` if there are active tasks.

**Parameters**
_None_

**Returns**
| Type | Description |
| :--- | :--- |
| `boolean` | Boolean indicating business. |

---

### `spawn(callback, args)`
Executes an asynchronous task.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `lambda` | `callback` | A lambda to invoke as a task. |
| `list` | `args` | A list of arguments for the callback. |

**Returns**
| Type | Description |
| :--- | :--- |
| `integer` | The task identifier. |

---

### `list()`
Returns a list of task identifiers.

**Parameters**
_None_

**Returns**
| Type | Description |
| :--- | :--- |
| `list` | A list of task identifiers. |

---

### `result(task_id)`
Returns the result of a task.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `task_id` | The task identifier. |

**Returns**
| Type | Description |
| :--- | :--- |
| `any` | The task result. |

---

### `sleep(ms)`
Sleeps for a number of milliseconds.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `ms` | The number of milliseconds to sleep. |

**Returns**
_None_

---

### `status(task_id)`
Returns the status of a given task.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `task_id` | The task identifier. |

**Returns**
| Type | Description |
| :--- | :--- |
| `string` | The status of the task as a string. |

---

### `wait()`
Waits for all tasks to complete.

**Parameters**
_None_

**Returns**
_None_