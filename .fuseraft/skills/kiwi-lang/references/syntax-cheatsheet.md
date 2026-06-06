# Kiwi Syntax Cheatsheet

Compact reference for the most common language constructs.

---

## Functions

```kiwi
fn name(param1, param2 = default, *rest)
  # body — last expression is returned
  return early_value when condition
  value
end

fn add(a: integer, b: integer): integer
  a + b
end

# Named arguments (order-independent)
add(b = 3, a = 7)

# Variadic + splat
fn sum(*args) ... end
sum(*[1, 2, 3])

# Hoisted at top level; nested functions are block-scoped
```

---

## Structs

```kiwi
struct Name < Parent
  static @shared = 0

  fn new(arg)
    @field = arg
    @@shared += 1
  end

  fn method() ... end
  static fn static_method() ... end
  private fn helper() ... end

  fn to_string() ... end   # custom string representation
end

# Abstract struct (cannot be instantiated directly)
abstract struct Shape
  fn area(): float   # abstract method, must be implemented
end

struct Circle < Shape
  fn new(r) @r = r end
  fn area() 3.14 * @r * @r end
end

obj = Name.new("value")
obj.method()
Name.static_method()
obj.is_a(Parent)           # type check
obj.keys() / obj.values()  # member introspection
obj.set("field", val)      # dynamic member set
obj.clone()

# super calls
super.new(...)             # constructor chain
super.method(...)          # method chain
```

---

## Error Handling

```kiwi
try
  risky()
catch e
  # e = { error: "Type", message: "..." }
  error::type_of(e)
  error::message_of(e)
  error::is_type(e, "KeyError")
catch t, m
  # t = type string, m = message string
finally
  cleanup()
end

throw error::ValueError("bad value")
throw "plain string"                    # becomes KiwiError

error::assert(x > 0, "must be positive")
error::expect_type(val, "integer")
error::expect_not_null(val)

# Re-throw
throw e
```

Named error constructors: `ArgumentError`, `AssertionError`, `IndexError`, `IOError`, `KeyError`, `NetworkError`, `NotFoundError`, `NotImplementedError`, `OverflowError`, `ParseError`, `PermissionError`, `TimeoutError`, `TypeError`, `ValueError`.

---

## Loops

```kiwi
repeat 5 do ... end
repeat 10 as i do ... end          # i = 1..10

while condition do
  break when guard
  next when guard
end

for item in collection do ... end
for item, idx in list do ... end   # idx starts at 0
for k, v in hashmap do ... end
for x in [1 to 10] do ... end
for x in generator() do ... end

break / next                       # with optional `when` guard
```

---

## Conditionals

```kiwi
if cond
  ...
elsif other
  ...
else
  ...
end

result = if x > 0 then "pos" else "neg" end

# when guards (expression modifiers)
return x * 2 when x > 0
break when i > 10
next when skip?
throw error::ValueError("x") when x <= 0

# case / when
case value
when 1..9:   "single"
when 10, 20: "special"
else:        "other"
end
```

Truthy: non-zero numbers, non-empty strings/lists/hashmaps, `true`.  
Falsey: `0`, `""`, `[]`, `{}`, `false`, `null`.

---

## Scoping

- **Global**: top-level declarations visible everywhere.
- **Function**: each call has its own scope; lexical lookup.
- **Block**: `if`/`while`/`for`/`repeat`/`do` create local scope. Variables assigned inside are block-local unless they already exist outside.
- **Instance**: `@field` (per instance), `@@static` (shared).
- **Package**: `pkg::VAR` for package-level variables.
- **Lambda**: captures caller's scope at invocation time.

Declare before a block if you need the value after it ends.

---

## Packages

```kiwi
package name
  fn func() ... end
  HOST = "localhost"
  const MAX = 100
end

export "name"          # inside the package file

pkg = import "name"    # in a consumer (string argument required)
pkg.func()

name::func()           # direct qualified access (if exported)

# Nested
package app::utils ... end
app::utils::func()

# Type extension (auto method)
package list
  fn sum_positive(_list: list) ... end
end
[1, -2, 3].sum_positive()
```

Standard library packages are pre-exported.

---

## Lambdas

```kiwi
# Arrow form (single expression)
do (x) => x * 2
do (a, b) => a + b end

# Block form
do (x)
  x * 2
end

# Assigned
double = do (x) => x * 2 end
double(5)

# Variadic
sum = with (*args) do ... end

# Passed to higher-order methods
[1,2,3].map(do (n) => n * 2)
[1,2,3].filter(do (n) => n > 0)
list.each(do (item) => ...)

# Named function as lambda (no conversion needed)
fn double(x) ... end
list.map(double)
```

---

## Generators

```kiwi
fn countdown(n)
  while n >= 0 do
    yield n
    n -= 1
  end
end

for x in countdown(5) do ... end
g = countdown(5)
typeof(g)  # "generator"

# Infinite
fn fib()
  a = 0; b = 1
  while true do
    yield a
    a, b = b, a + b
  end
end

for f in fib() do
  break when f > 100
end
```

---

## Ranges

```kiwi
[1 to 5]           # [1, 2, 3, 4, 5]  (list)
[10..1]            # countdown list
[1..5]             # inclusive

# In case (no list allocation)
case n
when 1..9:   "digit"
when 10..99: "double"
end

for i in [1 to 10] do ... end
```