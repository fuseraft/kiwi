# `task`

The `task` package provides functionality for managing and querying asynchronous tasks in Kiwi, including delayed and interval-based task execution.

## Table of Contents

- [Package Functions](#package-functions)
  - [`busy()`](#busy)
  - [`list()`](#list)
  - [`result(task_identifier)`](#resulttask_identifier)
  - [`sleep(ms)`](#sleepms)
  - [`status(task_identifier)`](#statustask_identifier)
  - [`timer(ms, callback)`](#timerms-callback)
  - [`interval(ms, callback)`](#intervalms-callback)
  - [`wait()`](#wait)
- [Creating Tasks with `fork`](#creating-tasks-with-fork)
- [Usage Examples](#usage-examples)

## Package Functions

### `busy()`

Checks if there are any active tasks.

**Returns**
| Type    | Description                       |
| :------ | :-------------------------------- |
| `Boolean`  | `true` if there are active tasks, `false` otherwise. |

### `list()`

Returns a list of active task identifiers.

**Returns**
| Type   | Description                 |
| :----- | :-------------------------- |
| `List` | List of integer task identifiers for active tasks. |

### `result(task_identifier)`

Fetches the result of a completed task, or returns a status hash if the task is still running.

**Parameters**
| Type      | Name             | Description                    |
| :-------- | :--------------- | :----------------------------- |
| `Integer` | `task_identifier` | The identifier of the task.    |

**Returns**
| Type     | Description |
| :------- | :---------- |
| `String` | The result of the task if completed, or a hash with the status if still running. |

### `sleep(ms)`

Pauses execution for a specified number of milliseconds.

**Parameters**
| Type      | Name | Description                           |
| :-------- | :--- | :------------------------------------ |
| `Integer` | `ms` | Number of milliseconds to pause.      |

### `status(task_identifier)`

Gets the current status of a specified task.

**Parameters**
| Type      | Name             | Description               |
| :-------- | :--------------- | :------------------------ |
| `Integer` | `task_identifier` | The identifier of the task.|

**Returns**
| Type   | Description               |
| :----- | :------------------------ |
| `Hash` | A hash containing the task's current status. |

### `timer(ms, callback)`

Schedules a one-time task to run after a specified delay.

**Parameters**
| Type      | Name      | Description                           |
| :-------- | :-------- | :------------------------------------ |
| `Integer` | `ms`      | Number of milliseconds to delay.      |
| `Lambda`  | `callback`| A lambda to invoke after the delay.   |

**Returns**
| Type     | Description                     |
| :------- | :------------------------------ |
| `Integer` | Task identifier of the scheduled task. |

### `interval(ms, callback)`

Schedules a task to run repeatedly at a specified interval until a condition is met.

**Parameters**
| Type      | Name      | Description                                                                 |
| :-------- | :-------- | :-------------------------------------------------------------------------- |
| `Integer` | `ms`      | Number of milliseconds between each execution.                              |
| `Lambda`  | `callback`| A lambda to invoke repeatedly. The task stops if `callback` returns `false`.|

**Returns**
| Type     | Description                     |
| :------- | :------------------------------ |
| `Integer` | Task identifier of the scheduled task. |

### `wait()`

Waits for all active tasks to complete before continuing execution.

**Returns**
| Type | Description |
| :--- | :---------- |
| `Void` | Blocks until all active tasks are completed. |

## Creating Tasks with `fork`

You can use the `fork` keyword in Kiwi to run a function asynchronously. When you `fork` a function, it immediately returns a task identifier that can be used to monitor and retrieve the result of the task once completed.

```kiwi
fn long_running_task()
  # simulate long computation
  task::sleep(1000)
  return "Task completed"
end

task_id = fork long_running_task()

println "Task started with ID: ${task_id}"
```

The `fork` keyword makes it easy to parallelize tasks or perform asynchronous operations without blocking the main thread.

## Usage Examples

### Checking for Active Tasks

You can check if there are any tasks currently running with `busy()`:

```kiwi
if task::busy()
  println("There are active tasks.")
end
```

### Listing Active Tasks

To get a list of all active task identifiers:

```kiwi
active_tasks = task::list()
println "Active tasks: ${active_tasks}"
```

### Fetching Task Results

Retrieve the result of a specific task or check its status:

```kiwi
result = task::result(1)

if result.status == "running"
  println "Task is still running."
else
  println "Task result: ${result}"
end
```

### Pausing Execution

Pause the execution for a set duration (in milliseconds):

```kiwi
task::sleep(1000)  # Pauses for 1 second
```

### Getting Task Status

Retrieve a status hash for a task:

```kiwi
status = task::status(1)
println "Task status: ${status["status"]}"
```

### Scheduling a Delayed Task with `timer`

Use `timer` to run a function after a specified delay:

```kiwi
task::timer(2000, (with do
  println "This runs after 2 seconds!"
end))
```

### Running a Task at Regular Intervals with `interval`

Use `interval` to run a function repeatedly with a specified delay between each execution. It stops when the callback returns `false`.

```kiwi
counter = 0
task::interval(500, (with do
  counter += 1
  println "Running task every half-second: ${counter}"
  return counter < 5  # Stops after 5 executions
end))
```

### Waiting for All Tasks to Complete with `wait`

To ensure all tasks finish before moving forward, use `wait`:

```kiwi
# Wait for all tasks to complete
task::wait()
println "All tasks completed."
```