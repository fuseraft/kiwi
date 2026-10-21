# Events

> The built-in event bus: on, once, emit and off, with wildcards and priorities.

Kiwi has a built-in event bus. Code registers handlers for named events with `on`, and other code triggers them with `emit`. The two sides don't need to know about each other, which makes events a good way to decouple parts of a program.

## Handling and emitting

```kiwi
on "user.login" with (user) do
  println "Welcome back, ${user["name"]}!"
end

emit "user.login" ({name: "Ada"})   # Welcome back, Ada!
```

- `on "event" do ... end` registers a handler with no parameters.
- `on "event" with (a, b) do ... end` registers a handler that receives arguments.
- `emit "event" (arg1, arg2)` runs every matching handler with those arguments. Leave the arguments off to emit without any.

Any number of handlers can listen for the same event. `emit` returns a list of their return values:

```kiwi
on "price" with (amount) do
  amount * 1.2
end

on "price" with (amount) do
  amount * 0.9
end

println emit "price" (100)   # [120, 90]
```

## One-time handlers

`once` registers a handler that runs on the first matching `emit` only:

```kiwi
once "ready" do
  println "initialized"
end

emit "ready"   # initialized
emit "ready"   # (nothing)
```

## Priorities

An integer after the event name sets a handler's priority. Higher priorities run first; the default is `0`. Handlers with equal priority run in the order they were registered.

```kiwi
on "request" do
  println "handle"
end

on "request", 10 do
  println "authenticate"
end

emit "request"
# authenticate
# handle
```

## Wildcards

`*` in an event name matches any text within one dot-separated segment.

```kiwi
on "user.*" with (name) do
  println "user event: ${name}"
end

on "*.error" with (message) do
  println "error: ${message}"
end

emit "user.login" ("ada")      # user event: ada
emit "db.error" ("timeout")    # error: timeout
emit "user.profile.updated" ("ada")   # (no match: * doesn't span dots)
```

## Removing handlers

`off "event"` removes every handler registered under that name. To remove one handler, register it as a variable and pass it to `off`:

```kiwi
log_it = do (x) => println "got ${x}"

on "data" (log_it)
emit "data" (1)        # got 1

off "data" (log_it)
emit "data" (2)        # (nothing)
```

## Errors in handlers

If a handler throws, the remaining handlers still run. Afterwards, `emit` throws a single `EventError` describing what failed.

```kiwi
on "tick" do
  throw "broken handler"
end

on "tick" do
  println "still runs"
end

try
  emit "tick"
catch e
  println e["error"]     # EventError
  println e["message"]   # Event handler error (tick): broken handler
end
```
