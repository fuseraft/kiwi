# Events

Kiwi provides a lightweight, zero-allocation, built-in event bus.

# Table of Contents
- [Event Keywords](#event-keywords)
  - [`on`](#on)
  - [`once`](#once)
  - [`emit`](#emit)
  - [`off`](#off)
- [Event Arguments](#event-arguments)
- [Example](#example)

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

Multiple handlers can listen to the same event and will all run in the order they were registered.

### `once`

Registers a handler that runs **only the first time** the event is emitted.

```kiwi
once "db.connected" do
  println "connected to the database"
end
```

After the first `emit`, the handler is automatically removed.

### `emit`

Triggers an event.

```kiwi
emit "app.start"

# pass parameters as you would with any function
emit "user.login" ({ "name": "alice", "role": "admin" })
```

### `off`

Removes event handlers.

| Syntax | Effect |
|---|---|
| `off "event.name"` | Removes **all** handlers for an event |
| `off "event.name" (handler)` | Removes a specific handler for an event |

```kiwi
my_handler = with (x) do println "Got ${x}" end

on "data" (my_handler)
emit "data" (42)          # prints: 42

off "data" (my_handler)
emit "data" (99)          # silent
```

## Event Arguments

```kiwi
on "log" with (level, message) do
  println "[${level}] ${message}"
end

emit "log" ("INFO", "Server started")
emit "log" ("ERROR", "Disk full!")
```

## Example

```kiwi
on "string-repeat" with (text: string, count: integer) do
  println text * count
end

repeat 5 as i do
  emit "string-repeat", "kiwi", i
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