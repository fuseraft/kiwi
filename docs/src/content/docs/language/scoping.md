---
title: Scoping
description: How Kiwi resolves names in scripts, functions, blocks, closures, structs and packages.
---

Kiwi has two main levels of scope: the **global** scope of the script, and the **local** scope of each function call. Blocks such as `if` and `for` don't introduce a scope of their own.

## Global variables

Variables assigned at the top level of a script are global. Functions can read them, and assigning to a global's name inside a function updates the global.

```kiwi
count = 0

fn increment()
  count += 1
end

increment()
increment()
println count   # 2
```

## Local variables

A variable first assigned inside a function, when no global of that name exists, is local to that call. It disappears when the function returns, and each call gets a fresh one.

```kiwi
fn make_greeting(name)
  message = "Hello, ${name}"
  message
end

println make_greeting("Kiwi")   # Hello, Kiwi
```

```kiwi fails
fn setup()
  temp = 1
end

setup()
println temp   # VariableUndefinedError: temp only existed inside setup()
```

Parameters are always local, even when a global has the same name.

### Shadowing a global with `var`

Because plain assignment updates a global of the same name, use `var` inside a function to declare a local variable that shadows it:

```kiwi
name = "global"

fn rename()
  var name = "local"
  name
end

println rename()   # local
println name       # global
```

This matters most in long scripts where a helper function might reuse a common name such as `result` or `i` that also exists at the top level.

## Blocks don't create scope

`if`, `case`, `for`, `while`, `repeat` and `try` blocks share the scope around them. A variable assigned inside a block is still visible after it, and loop variables keep their last value.

```kiwi
if true
  status = "ready"
end
println status   # ready

for n in [1, 2, 3] do
  last_seen = n
end
println last_seen   # 3
println n           # 3
```

A variable assigned only in a branch that didn't run doesn't exist. When a value is computed in different branches, initialize it first so that it is always defined:

```kiwi
score = 72
grade = "F"

if score >= 90
  grade = "A"
elsif score >= 70
  grade = "C"
end

println grade   # C
```

## Closures

A [lambda](/kiwi/language/lambdas/) or nested function can use the local variables of the function it was created in, and those variables live as long as the lambda does:

```kiwi
fn make_counter()
  n = 0
  do ()
    n += 1
    n
  end
end

tick = make_counter()
tick()
println tick()   # 2
```

## Struct instances

Inside a struct method, `@name` refers to an instance variable. Instance variables belong to the object, not to any lexical scope, and each instance has its own. `@@name` refers to a static variable shared by every instance of the struct. See [Structs](/kiwi/language/structs/).

```kiwi
struct Counter
  static @created = 0

  fn new()
    @value = 0
    @@created += 1
  end

  fn tick()
    @value += 1
  end
end

a = Counter.new()
b = Counter.new()
a.tick()
println [a.value, b.value]   # [1, 0]
println Counter.created      # 2
```

## Packages

Variables assigned at the top of a `package` body are package variables, reached from outside as `package::NAME`. Functions in the same package call each other without the prefix. See [Packages](/kiwi/language/packages/#package-variables).

```kiwi
package settings
  LEVEL = "debug"

  fn describe()
    "level=${settings::LEVEL}"
  end
end

println settings::LEVEL        # debug
println settings::describe()   # level=debug
```
