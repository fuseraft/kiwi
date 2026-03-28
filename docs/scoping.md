# Scoping

Kiwi uses **lexical scoping** — each name is resolved in the scope where it was written, not where it was called. There are four scope levels: global, function, block, and instance/struct.

---

## Global Scope

Variables declared at the top level of a script are globally visible and can be read or updated from inside any function.

```kiwi
count = 0

fn increment()
  count += 1
end

increment()
increment()
println count  # 2
```

---

## Function Scope

Each function call gets its own scope, parented to the scope where the function was **defined** (not where it was called). A new variable created inside a function is local to that call and disappears when the function returns.

```kiwi
fn make_counter(start)
  n = start
  fn step()
    n += 1
    n
  end
  step
end

c = make_counter(0)
println c()  # 1
println c()  # 2
```

Reading an outer variable works because scope lookup walks up the chain. Writing to an outer name updates the existing binding; writing a **new** name creates a local variable.

### Nested Functions

A `fn` declaration inside another function is local to that function. It is callable anywhere within the enclosing function body — including after `try`/`catch` blocks — but ceases to exist when the function returns.

```kiwi
fn outer()
  fn helper()
    return 42
  end
  return helper()  # ok
end

outer()
helper()  # error — helper is not defined here
```

Lambda aliases assigned inside a function follow the same rule:

```kiwi
fn run()
  double = with (x) do x * 2 end
  return double(5)  # 10
end

run()
double(3)  # error — double is not defined here
```

```kiwi
x = 10

fn double_x()
  x = x * 2  # updates outer x because x exists there
end

double_x()
println x  # 20
```

```kiwi
x = 10

fn shadow()
  y = 99  # y does not exist outside — stays local
end

shadow()
println y  # null
```

---

## Block Scope

`if`, `elsif`, `else`, `while`, `for`, `repeat`, and `do` blocks each introduce their own scope. A variable first assigned **inside** a block is local to that block and is not visible after it ends.

```kiwi
if true
  msg = "hello"
end

println msg  # null — msg was block-local
```

Assigning to a variable that already exists **outside** the block still updates the outer binding normally.

```kiwi
result = null

if true
  result = "computed"  # updates outer result
end

println result  # computed
```

### Declare before the block

The common pattern for "compute a value in one branch, use it after" is to declare the variable before the block:

```kiwi
label = ""

if score >= 90
  label = "A"
elsif score >= 80
  label = "B"
else
  label = "C"
end

println "Grade: ${label}"
```

### Loop variables are block-scoped

Iterator variables (`v`, `i` in a `for` loop, the alias in `repeat`) and any variables declared in a loop body are local to that loop. They are not visible after the loop ends.

```kiwi
for x in [1, 2, 3] do
  last = x
end

println x     # null
println last  # null
```

To retain a value across the loop boundary, declare it before the loop:

```kiwi
last = null

for x in [1, 2, 3] do
  last = x
end

println last  # 3
```

---

## Instance Scope

Inside a struct method, instance variables are accessed with `@`. They live on the object, not in any lexical scope, and persist for the lifetime of the instance.

```kiwi
struct Counter
  fn new(start: integer = 0)
    @n = start
  end

  fn tick()
    @n += 1
  end

  fn value(): integer
    @n
  end
end

c = Counter.new()
c.tick()
c.tick()
println c.value()  # 2
```

Each instance has its own copy of `@n`. Two `Counter` objects do not share state.

### Static Variables

Static variables are declared with `static @name` in the struct body and accessed inside methods with `@@name`. They are shared across all instances.

```kiwi
struct IdGen
  static @next = 1

  fn new()
    @id = @@next
    @@next += 1
  end
end

a = IdGen.new()
b = IdGen.new()
println a.id  # 1
println b.id  # 2
```

See [Structs](structs.md) for full details on instance and static variables.

---

## Package Scope

Functions defined inside a `package` block are accessed with `::` after the package is exported. They run in their own function scope and cannot see variables from the call site.

```kiwi
package math_utils
  fn square(n: integer): integer
    n * n
  end
end

export "math_utils"

println math_utils::square(5)  # 25
```

See [Packages](packages.md) for details.

---

## Lambda and Closure Scope

A lambda defined with `do` or `with` captures the **caller's scope** at the point of invocation. This lets it read and update local variables.

```kiwi
total = 0
items = [1, 2, 3, 4, 5]

items.each(do (n) => total += n)

println total  # 15
```

A lambda converted from a named function with `.to_lambda()` runs in its own isolated function scope (same rules as a regular function call).

See [Lambdas](lambdas.md) for full details.
