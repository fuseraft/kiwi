# `task`

The `task` package provides functionality for managing and querying asynchronous tasks in Kiwi.

## Table of Contents

- [Package Functions](#package-functions)
  - [`busy()`](#busy)
  - [`list()`](#list)
  - [`result(task_identifier)`](#resulttask_identifier)
  - [`sleep(ms)`](#sleepms)
  - [`status(task_identifier)`](#statustask_identifier)
- [Creating Tasks with `fork`](#creating-tasks-with-fork)
- [Usage Examples](#usage-examples)

## Package Functions

### `busy()`

Checks if there are any active tasks.

**Returns**
| Type    | Description                       |
| :------ | :-------------------------------- |
| `Bool`  | `true` if there are active tasks, `false` otherwise. |

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