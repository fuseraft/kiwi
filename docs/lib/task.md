# `task`

The `task` package contains functionality for concurrency in Kiwi.

## Table of Contents

- [Package Functions](#package-functions)
  - [`await(task_id)`](#awaittask_id)
  - [`busy()`](#busy)
  - [`channel(cap)`](#channelcap)
  - [`interval(ms, callback)`](#intervalms-callback)
  - [`spawn(callback, args)`](#spawncallback-args)
  - [`list()`](#list)
  - [`result(task_id)`](#resulttask_id)
  - [`sleep(ms)`](#sleepms)
  - [`status(task_id)`](#statustask_id)
  - [`timer(ms, callback)`](#timerms-callback)
  - [`wait()`](#wait)
- [`Channel` struct](#channel)
  - [Constructor](#constructor)
  - [`close()`](#close)
  - [`closed()`](#closed)
  - [`recv()`](#recv)
  - [`send(data)`](#senddata)
  - [`try_recv()`](#try_recv)

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
| `boolean` | `true` if there are active tasks. |

---

### `channel(cap)`
Creates and returns a new `Channel` with the given capacity.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `cap` | Channel capacity. `0` means unbounded. |

**Returns**
| Type | Description |
| :--- | :--- |
| `Channel` | A new channel instance. |

---

### `interval(ms, callback)`
Spawns a background task that calls `callback` repeatedly every `ms` milliseconds. Runs indefinitely until the task is cancelled or the program exits.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `ms` | Interval duration in milliseconds. |
| `lambda` | `callback` | The lambda to invoke on each interval tick. |

**Returns**
| Type | Description |
| :--- | :--- |
| `integer` | The task identifier of the spawned interval task. |

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
| `hashmap` | A hashmap containing task status information. |

---

### `timer(ms, callback)`
Waits `ms` milliseconds and then invokes `callback` once. Blocks until the timer completes.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `ms` | Delay in milliseconds before invoking the callback. |
| `lambda` | `callback` | The lambda to invoke after the delay. |

**Returns**
_None_

---

### `wait()`
Waits for all tasks to complete.

**Parameters**
_None_

**Returns**
_None_

---

## `Channel`

A synchronization structure for asynchronous producer-consumer patterns, acting as a thread-safe queue to pass data between concurrent tasks, ensuring items are processed in FIFO order.

### Constructor

Creates a channel. When `capacity` is `0`, the channel is "unbounded." 

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `capacity` | The number of items the channel may store. |

### `close()`

Closes the channel.

**Parameters**
_None_

### `closed()`

Returns `true` if the channel is closed.

**Parameters**
_None_

**Returns**
| Type | Description |
| :--- | :--- |
| `boolean` | `true` when the channel is closed. |

### `recv()`

Reads an item from the channel.

**Parameters**
_None_

**Returns**
| Type | Description |
| :--- | :--- |
| `any` | The item. |

### `send(data)`

Writes data to a channel.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `data` | An item to store on the channel. |

### `try_recv()`

Attempts to receive an item from the channel. 

**Parameters**
_None_

**Returns**
| Type | Description |
| :--- | :--- |
| `list` | Returns a list containing two values: a `boolean` indicating whether a value was received, and the value if received (otherwise `0`).  |
