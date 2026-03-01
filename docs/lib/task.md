# `task`

The `task` package contains functionality for concurrency in Kiwi.

---

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

**Example**

```kiwi
import "task"

# Spawn a background computation and await its result
worker = do (n: integer)
  total = 0
  for i in [1 to n] do
    total += i
  end
  total
end

id = task::spawn(worker, [1000])
result = task::await(id)
println "Sum 1..1000 = ${result}"
# => Sum 1..1000 = 500500
```

---

### `busy()`
Returns `true` if there are active tasks.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if there are active tasks. |

**Example**

```kiwi
import "task"

worker = do (ms: integer)
  task::sleep(ms)
  "done"
end

id = task::spawn(worker, [200])
println "Busy right after spawn: ${task::busy()}"  # => true
task::await(id)
println "Busy after await: ${task::busy()}"        # => false
```

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

**Example**

```kiwi
import "task"

# Producer/consumer connected through a bounded channel (capacity 3)
ch = task::channel(3)

producer = with (chan: Channel) do
  for i in [1 to 5] do
    println "[Producer] sending ${i}"
    chan.send(i)
  end
  chan.close()
  println "[Producer] done"
end

consumer = with (chan: Channel) do
  while v = chan.recv() do
    println "[Consumer] received ${v}"
    task::sleep(50)
  end
  println "[Consumer] channel closed"
end

task::spawn(producer, [ch])
task::spawn(consumer, [ch])
task::wait()
```

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

**Example**

```kiwi
import "task"
import "time"

# Tick a counter every 200ms, let it run for 3 ticks then stop
count = 0

ticker_id = task::interval(200, with () do
  count += 1
  println "Tick ${count} at ${time::timestamp()}"
end)

# Wait long enough for ~3 ticks
task::sleep(700)

println "Final count: ${count}"
# The interval task keeps running until the program exits.
# Use task::list() and task::result() to inspect running tasks.
println "Running tasks: ${task::list()}"
```

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

**Example**

```kiwi
import "task"

fn slow_square(n: integer): integer
  task::sleep(100)  # simulate work
  n * n
end

worker = do (n: integer)
  slow_square(n)
end

# Spawn several tasks in parallel
ids = []
for n in [2, 4, 6, 8] do
  ids.append(task::spawn(worker, [n]))
end

# Collect all results
for id, idx in ids do
  result = task::await(id)
  println "Result ${idx}: ${result}"
end
# => Result 0: 4
# => Result 1: 16
# => Result 2: 36
# => Result 3: 64
```

---

### `list()`
Returns a list of task identifiers.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of task identifiers. |

**Example**

```kiwi
import "task"

worker = do (ms: integer)
  task::sleep(ms)
  ms
end

task::spawn(worker, [300])
task::spawn(worker, [400])
task::spawn(worker, [500])

println "Active task ids: ${task::list()}"
# => Active task ids: [1, 2, 3]

task::wait()
println "Active task ids after wait: ${task::list()}"
# => Active task ids after wait: []
```

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

**Example**

```kiwi
import "task"

worker = do (x: integer)
  task::sleep(50)
  x * 10
end

id = task::spawn(worker, [7])
task::await(id)

# task::result returns the stored result without blocking
println task::result(id)
# => 70
```

---

### `sleep(ms)`
Sleeps for a number of milliseconds.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `ms` | The number of milliseconds to sleep. |

**Returns**
_None_

**Example**

```kiwi
import "task"
import "time"

# Pause 100ms between polling attempts
attempts = 0
while attempts < 3 do
  attempts += 1
  println "Attempt ${attempts} at ${time::timestamp("HH:mm:ss")}"
  task::sleep(100)
end
```

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

**Example**

```kiwi
import "task"

worker = do (ms: integer)
  task::sleep(ms)
  "finished"
end

id = task::spawn(worker, [300])

# Check status while the task is still running
task::sleep(50)
s = task::status(id)
println "Status: ${s}"

task::await(id)
s = task::status(id)
println "Status after completion: ${s}"
```

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

**Example**

```kiwi
import "task"
import "time"

println "Waiting 250ms for delayed callback..."

task::timer(250, with () do
  println "Timer fired at ${time::timestamp("HH:mm:ss")}"
end)

println "Timer complete — execution continues here"
```

---

### `wait()`
Waits for all tasks to complete.

**Parameters**
_None_

**Returns**
_None_

**Example**

```kiwi
import "task"

worker = do (label: string, ms: integer)
  task::sleep(ms)
  println "${label} done"
end

task::spawn(worker, ["alpha", 200])
task::spawn(worker, ["beta",  150])
task::spawn(worker, ["gamma", 300])

println "All tasks spawned. Waiting..."
task::wait()
println "All tasks finished."
```

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

**Example**

```kiwi
import "task"

ch = task::channel(4)

# Fill the channel with some values
for i in [1 to 4] do
  ch.send(i * 10)
end
ch.close()

# Non-blocking drain via try_recv
while true do
  pair = ch.try_recv()
  ok   = pair[0]
  val  = pair[1]
  break when !ok
  println "got: ${val}"
end
println "Channel closed: ${ch.closed()}"
# => got: 10
# => got: 20
# => got: 30
# => got: 40
# => Channel closed: true
```
