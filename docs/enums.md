# Enums

Enums define a named set of constant values. They are useful for representing a fixed collection of related states, codes, or categories.

---

## Defining an Enum

Use the `enum` keyword followed by a name and a block of member declarations, closed with `end`.

```kiwi
enum Direction
  North
  East
  South
  West
end
```

Members are assigned integer values starting at `0`, incrementing by `1` for each subsequent member.

## Accessing Members

Access enum members with dot notation: `EnumName.MemberName`.

```kiwi
d = Direction.North
println d              # 0
println Direction.East # 1
```

## Explicit Values

Assign an explicit value to a member with `=`. Auto-increment resumes from that value for subsequent members.

```kiwi
enum HttpStatus
  OK        = 200
  Created   = 201
  NotFound  = 404
  Error     = 500
end

println HttpStatus.OK       # 200
println HttpStatus.NotFound # 404
```

```kiwi
enum Priority
  Low           # 0
  Medium        # 1
  High   = 10
  Critical      # 11
end

println Priority.High     # 10
println Priority.Critical # 11
```

## Comparing Enum Values

Since members are plain values (integers by default), use `==` to compare them.

```kiwi
enum Status
  Pending
  Active
  Inactive
end

s = Status.Active

if s == Status.Active
  println "active"
end
```

## Non-Integer Values

Enum members can hold any value type, not just integers.

```kiwi
enum Color
  Red   = "red"
  Green = "green"
  Blue  = "blue"
end

println Color.Red  # red
```

## Using Enums in Collections

```kiwi
enum Level
  Debug = 0
  Info  = 1
  Warn  = 2
  Error = 3
end

log_levels = [Level.Info, Level.Warn, Level.Error]
println log_levels  # [1, 2, 3]
```
