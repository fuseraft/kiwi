# Events

Kiwi has a built-in event bus with wildcard routing, handler priority, and error isolation.

## Event Keywords

### `on`

Registers a handler that runs **every time** the event is emitted.

```kiwi
on "app.start" do
  println "Application is starting..."
end

on "user.login" with (user) do
  println "Welcome back, ${user.name}!"
end
```

Multiple handlers can listen to the same event and fire in priority order (highest first),
then registration order within the same priority.

#### Priority

An optional integer after the event name controls dispatch order. Higher values run first.
The default priority is `0`.

```kiwi
on "request", 10 do   # runs first
  println "auth check"
end

on "request" do       # runs second (priority 0)
  println "handle request"
end
```

#### Wildcard patterns

`*` matches any sequence of non-`.` characters within a single segment.

```kiwi
on "user.*" with (name) do
  println "user event for ${name}"
end

on "*.error" with (msg) do
  println "error: ${msg}"
end

emit "user.login"  ("alice")   # matches "user.*"
emit "db.error"    ("timeout") # matches "*.error"
emit "user.x.y"    ("bob")     # no match — "*" doesn't cross "."
```

### `once`

Registers a handler that runs **only the first time** the event is emitted.
The handler is removed before it executes, so it will not fire again even if it throws.

```kiwi
once "db.connected" do
  println "connected to the database"
end
```

Accepts the same priority and wildcard syntax as `on`.

### `emit`

Triggers an event and returns a list of all handler return values.

```kiwi
emit "app.start"

# pass arguments
emit "user.login" ({ "name": "alice", "role": "admin" })

# capture return values
on "calc" with (x) do
  return x * 2
end

results = emit "calc" (21)   # results == [42]
```

#### Error isolation

If a handler throws, the remaining handlers still run. All errors are collected and
re-thrown as a single error after the full dispatch round completes.

```kiwi
on "tick" do
  throw "oops"
end

on "tick" do
  println "I still run"
end

try
  emit "tick"
catch (err)
  println err   # reports the collected error
end
```

### `off`

Removes event handlers.

| Syntax | Effect |
|---|---|
| `off "event.name"` | Removes **all** handlers for that pattern |
| `off "event.name" (handler)` | Removes a specific handler |

```kiwi
my_handler = do (x) => println "Got ${x}"

on "data" (my_handler)
emit "data" (42)          # prints: Got 42

off "data" (my_handler)
emit "data" (99)          # silent
```

## Example

```kiwi
on "string-repeat" with (text: string, count: integer) do
  println text * count
end

repeat 5 as i do
  emit "string-repeat" ("kiwi", i)
end
```

### Output
```
kiwi
kiwikiwi
kiwikiwikiwi
kiwikiwikiwikiwi
kiwikiwikiwikiwikiwi
```
