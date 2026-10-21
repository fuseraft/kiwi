# Enums

> Named sets of constant values.

An enum defines a named set of constants, which is clearer than scattering magic numbers or strings through a program.

```kiwi
enum Direction
  North
  East
  South
  West
end

heading = Direction.East
println heading                     # 1
println heading == Direction.East   # true
```

## Values

Members are numbered from `0` in order. Give a member an explicit value with `=`; members after it continue counting from there.

```kiwi
enum Priority
  Low           # 0
  Medium        # 1
  High = 10
  Critical      # 11
end

println Priority.Medium     # 1
println Priority.Critical   # 11
```

Values can be any type, such as strings:

```kiwi
enum Color
  Red   = "#ff0000"
  Green = "#00ff00"
end

println Color.Green   # #00ff00
```

A member *is* its value. `Direction.East` is the integer `1`, so it compares, prints and serializes like one, and `typeof(Direction.East)` is `integer`.

## Using enums

Enums work well with `case`:

```kiwi
enum Status
  Pending
  Active
  Closed
end

fn label(status)
  case status
    when Status.Pending "waiting"
    when Status.Active  "in progress"
    else                "done"
  end
end

println label(Status.Active)   # in progress
```

## Enum methods

| Method | Returns |
| :-- | :-- |
| `.size()` | The number of members |
| `.keys()` | The member names, in order |
| `.values()` | The member values, in order |
| `.to_hashmap()` | A hashmap of names to values |
| `.get(name, default)` | The value of the member called `name`, or `default` |

```kiwi
enum HttpStatus
  OK = 200
  NotFound = 404
end

println HttpStatus.size()                 # 2
println HttpStatus.keys()                 # ["OK", "NotFound"]
println HttpStatus.to_hashmap()           # {"OK": 200, "NotFound": 404}
println HttpStatus.get("NotFound", 0)     # 404
println HttpStatus.get("Teapot", 418)     # 418

for name in HttpStatus.keys() do
  println "${name} = ${HttpStatus.get(name, null)}"
end
```

Without a default, looking up a missing name raises a `ValueError`:

```kiwi
enum HttpStatus
  OK = 200
end

try
  HttpStatus.get("Teapot")
catch e
  println e["message"]   # Enum member 'Teapot' not found.
end
```
