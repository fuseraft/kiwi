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

`task::sleep(ms)` pauses the current task. `task::timer(ms, lambda)` runs a lambda once after a delay, and `task::interval(ms, lambda)` runs it repeatedly:

```kiwi
fired = [0]
task::timer(20, do () => fired[0] += 1)
task::sleep(100)
println fired[0]   # 1
```

:::caution
An interval runs forever and can't be cancelled, so a program that starts one never exits on its own. Prefer a loop in a spawned task that checks a flag.
:::

## Inspecting tasks

`task::list()` returns the ids of running tasks, `task::busy()` whether any are running, `task::status(id)` a task's state, and `task::wait()` waits for all of them.

## Functions

### `task::await`

```kiwi
task::await(task_id: integer)
```

Await a specified task.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `task_id` | `integer` |  |  |

**Returns** The result of the task.

### `task::busy`

```kiwi
task::busy()
```

Returns true if there are active tasks.

**Returns** Boolean indicating business.

### `task::channel`

```kiwi
task::channel(cap: integer = 0): Channel
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `cap` | `integer` | `0` |  |

**Returns** `Channel`

### `task::interval`

```kiwi
task::interval(ms: integer, callback: lambda)
```

Execute a callback on an interval.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ms` | `integer` |  | An integer representing milliseconds to sleep. |
| `callback` | `lambda` |  | A lambda to invoke after sleeping. |

### `task::spawn`

```kiwi
task::spawn(callback: lambda, args: list = [])
```

Spawn a new task.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `callback` | `lambda` |  | A lambda to invoke as a task. |
| `args` | `list` | `[]` |  |

**Returns** integer containing the task identifier.

### `task::list`

```kiwi
task::list(): list
```

Returns a list of task identifiers.

**Returns** `list` — List of integers.

### `task::result`

```kiwi
task::result(task_identifier: integer)
```

Returns a result or a status hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `task_identifier` | `integer` |  | An integer task identifier. |

**Returns** Result of a task, or its status in a hashmap.

### `task::sleep`

```kiwi
task::sleep(ms: integer)
```

Sleeps for a number of milliseconds.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ms` | `integer` |  | An integer representing milliseconds to sleep. |

### `task::status`

```kiwi
task::status(task_identifier: integer)
```

Get a status hashmap for a given task.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `task_identifier` | `integer` |  | An integer task identifier. |

**Returns** Hashmap containing status.

### `task::timer`

```kiwi
task::timer(ms: integer, callback: lambda)
```

Execute a callback after a specified amount of milliseconds.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ms` | `integer` |  | An integer representing milliseconds to sleep. |
| `callback` | `lambda` |  | A lambda to invoke after sleeping. |

### `task::wait`

```kiwi
task::wait()
```

Waits for all tasks to complete.

### `task::mutex`

```kiwi
task::mutex()
```

Creates a new Mutex.

**Returns** Mutex

## `Channel` struct

Represents a channel

### `Channel.new`

```kiwi
Channel.new(cap: integer = 0)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `cap` | `integer` | `0` |  |

### `.close`

```kiwi
close()
```

### `.recv`

```kiwi
recv(): any
```

**Returns** `any`

### `.try_recv`

```kiwi
try_recv(): list
```

Attempts to receive data from the channel.

**Returns** `list` — [boolean, any] containing whether a value was received, and if so the value

### `.closed`

```kiwi
closed(): boolean
```

**Returns** `boolean`

### `.send`

```kiwi
send(data: any)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  |  |

## `Mutex` struct

A mutual exclusion lock backed by a semaphore.

### `Mutex.new`

```kiwi
Mutex.new()
```

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

### `.destroy`

```kiwi
destroy()
```

Destroys the mutex and frees resources.
