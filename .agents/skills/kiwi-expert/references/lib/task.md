# task

> Run code concurrently with tasks, and coordinate with channels and mutexes.

The `task` package runs code concurrently. `task::spawn` starts a lambda on a background thread and returns a task id; `task::await` waits for it and returns its result.

```kiwi
id = task::spawn(do (a, b) => a * b, [6, 7])
println task::await(id)   # 42
```

The list after the lambda holds its arguments. Start several tasks and await them all to do work in parallel:

```kiwi
ids = [1 to 3].map(do (n) => task::spawn(do (k)
  task::sleep(10 * k)
  k * k
end, [n]))

println ids.map(do (id) => task::await(id))   # [1, 4, 9]
```

An error thrown inside a task is rethrown by `task::await`:

```kiwi
try
  task::await(task::spawn(do () => throw "failed inside the task"))
catch e
  println e["message"]   # failed inside the task
end
```

A program doesn't exit until its tasks have finished. `spawn` and `await` are also available as the global functions [`spawn` and `await`](../reference/global-functions.md#tasks).

## Channels

A `Channel` passes values between tasks. `send` puts a value in, `recv` waits for the next one, and `close` signals that no more values are coming, after which `recv` returns `null`.

```kiwi
ch = task::channel()

producer = task::spawn(do (c)
  for n in [1 to 3] do
    c.send(n)
  end
  c.close()
end, [ch])

received = []
while true do
  item = ch.recv()
  break when item == null
  received.push(item)
end

println received   # [1, 2, 3]
task::await(producer)
```

`task::channel(capacity)` creates a buffered channel; with a capacity, `send` waits while the channel is full. `try_recv()` returns `[true, value]` if a value was ready and `[false, null]` otherwise, without waiting.

## Sharing state

A task's lambda shares variables with the code that started it the way any [closure](../language/lambdas.md#which-variables-a-lambda-shares) does: a variable assigned before the lambda is shared, and both sides see each other's assignments. A variable the lambda assigns that the code around it only assigns later is the task's own, so a task can reuse names such as `result` or `id` without touching the spawner's. Use `var` for a task variable whose name the spawner has already used.

Tasks can share lists, hashmaps and objects with the code that started them. Each single operation on a shared value, such as `push`, `xs[i] = v` or `m[k] = v`, is atomic, so concurrent tasks never corrupt it.

An update that reads and then writes, such as `count += 1`, is two operations, and two tasks can lose each other's updates. Guard it with a mutex:

```kiwi
counter = [0]
lock = task::mutex()

workers = [1 to 4].map(do (n) => task::spawn(do (c, m)
  repeat 500 do
    m.synchronize(do () => c[0] += 1)
  end
end, [counter, lock]))

workers.each(do (w) => task::await(w))
println counter[0]   # 2000
```

`synchronize(lambda)` holds the lock while the lambda runs. `acquire()`, `try_acquire(timeout_ms)` and `release()` give finer control.

Other points:

- List methods that take a lambda work on a snapshot of the list, so another task's changes don't affect an iteration already under way.
- Imports are serialized: if two tasks import the same package, the second waits until the first finishes.
- For handing work from one task to another, a channel is usually simpler than shared state.

## Timers

`task::sleep(ms)` pauses the current task. `task::timer(ms, lambda)` runs a lambda once after a delay and waits for it, and `task::interval(ms, lambda)` runs it repeatedly in the background:

```kiwi
fired = [0]
task::timer(20, do () => fired[0] += 1)
task::sleep(100)
println fired[0]   # 1
```

`interval` returns an `Interval`. Call `stop()` on it to end it, or return `false` from the lambda. A program waits for its running intervals before it exits, so stop one when it is no longer needed:

```kiwi
ticks = [0]
heartbeat = task::interval(10, do () => ticks[0] += 1)
task::sleep(55)
heartbeat.stop()
task::await(heartbeat.id())
println ticks[0] > 0   # true

countdown = [3]
task::await(task::interval(5, do ()
  countdown[0] -= 1
  countdown[0] > 0
end).id())
println countdown[0]   # 0
```

## Inspecting tasks

`task::list()` returns the ids of running tasks, `task::busy()` whether any are running, `task::status(id)` a task's state, and `task::wait()` waits for all of them.

## Functions

### `task::await`

```kiwi
task::await(task_id: integer)
```

Waits for a task to finish. An error thrown inside the task is rethrown here.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `task_id` | `integer` |  | The task identifier. |

**Returns** `any` — The result of the task.

### `task::busy`

```kiwi
task::busy()
```

Checks whether any tasks are running.

**Returns** `boolean` — `true` if there are active tasks.

### `task::channel`

```kiwi
task::channel(cap: integer = 0): Channel
```

Creates a channel.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `cap` | `integer` | `0` | The buffer capacity (default `0`, unbuffered). |

**Returns** `Channel` — The channel.

### `task::interval`

```kiwi
task::interval(ms: integer, callback: lambda): Interval
```

Calls a lambda repeatedly, waiting `ms` milliseconds between calls. A program waits for running intervals before it exits, so stop one when it is no longer needed.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ms` | `integer` |  | The milliseconds to wait between calls. |
| `callback` | `lambda` |  | The lambda to call. Returning `false` stops the interval. |

**Returns** `Interval` — The interval; call `stop()` to end it.

### `task::spawn`

```kiwi
task::spawn(callback: lambda, args: list = [])
```

Runs a lambda on a background task.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `callback` | `lambda` |  | The lambda to run. |
| `args` | `list` | `[]` | The arguments to pass to the lambda. |

**Returns** `integer` — The task identifier.

### `task::list`

```kiwi
task::list(): list
```

Gets the identifiers of the running tasks.

**Returns** `list` — The task identifiers.

### `task::result`

```kiwi
task::result(task_identifier: integer)
```

Gets a task's result without waiting.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `task_identifier` | `integer` |  | The task identifier. |

**Returns** `any` — The task's result if it has finished, otherwise a hashmap with its status.

### `task::sleep`

```kiwi
task::sleep(ms: integer)
```

Pauses the current task.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ms` | `integer` |  | The milliseconds to sleep. |

### `task::status`

```kiwi
task::status(task_identifier: integer)
```

Gets a task's status.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `task_identifier` | `integer` |  | The task identifier. |

**Returns** `hashmap` — The task's status.

### `task::timer`

```kiwi
task::timer(ms: integer, callback: lambda)
```

Calls a lambda once after a delay, and waits for it.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ms` | `integer` |  | The milliseconds to wait first. |
| `callback` | `lambda` |  | The lambda to call. |

### `task::wait`

```kiwi
task::wait()
```

Waits for all tasks to complete.

### `task::mutex`

```kiwi
task::mutex()
```

Creates a mutex.

**Returns** `Mutex` — The mutex.

## `Channel` struct

A channel for passing values between tasks.

### `Channel.new`

```kiwi
Channel.new(cap: integer = 0)
```

Creates a channel.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `cap` | `integer` | `0` | The buffer capacity; `0` (the default) makes `send` wait for a receiver. |

### `.close`

```kiwi
close()
```

Closes the channel. After it is drained, `recv` returns `null`.

### `.recv`

```kiwi
recv(): any
```

Waits for the next value.

**Returns** `any` — The value, or `null` once the channel is closed and empty.

### `.try_recv`

```kiwi
try_recv(): list
```

Receives a value if one is ready, without waiting.

**Returns** `list` — `[true, value]` if a value was received, otherwise `[false, null]`.

### `.closed`

```kiwi
closed(): boolean
```

Checks whether `close()` was called on this channel.

**Returns** `boolean` — `true` if the channel is closed.

### `.send`

```kiwi
send(data: any)
```

Sends a value, waiting while a buffered channel is full.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The value to send. |

## `Interval` struct

A repeating task started by `task::interval`.

### `Interval.new`

```kiwi
Interval.new(ms: integer, callback: lambda)
```

Starts calling `callback` every `ms` milliseconds on a background task.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ms` | `integer` |  | The milliseconds to wait between calls. |
| `callback` | `lambda` |  | The lambda to call. Returning `false` stops the interval. |

### `.stop`

```kiwi
stop()
```

Stops the interval. A call already running finishes, and no further calls start.

### `.id`

```kiwi
id(): integer
```

Gets the identifier of the interval's background task.

**Returns** `integer` — The task identifier.

## `Mutex` struct

A mutual exclusion lock backed by a semaphore.

### `Mutex.new`

```kiwi
Mutex.new()
```

Creates an unlocked mutex.

### `.acquire`

```kiwi
acquire()
```

Acquires the mutex, blocking until available.

**Returns** `Mutex` — self

### `.try_acquire`

```kiwi
try_acquire(timeout_ms = 0)
```

Tries to acquire the mutex without blocking (or with a timeout).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `timeout_ms` | `integer` | `0` | Milliseconds to wait (default 0 = no wait). |

**Returns** `boolean` — true if acquired.

### `.release`

```kiwi
release()
```

Releases the mutex.

**Returns** `Mutex` — self

### `.synchronize`

```kiwi
synchronize(callback)
```

Acquires the mutex, runs the callback, then releases it.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `callback` | `lambda` |  | Zero-argument lambda to execute in the critical section. |

**Returns** `any` — The callback's result.

### `.destroy`

```kiwi
destroy()
```

Destroys the mutex and frees resources.
