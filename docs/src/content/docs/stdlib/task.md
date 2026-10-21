---
title: task
description: Run code concurrently with tasks, and coordinate with channels and mutexes.
---

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

A program doesn't exit until its tasks have finished. `spawn` and `await` are also available as the global functions [`spawn` and `await`](/kiwi/reference/global-functions/#tasks).

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

<!-- api: task -->
