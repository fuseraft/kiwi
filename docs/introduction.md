# Introduction to Kiwi

Kiwi is a high-level, dynamically-typed scripting language designed to make everyday programming tasks feel natural and expressive. It draws inspiration from Ruby's elegance and Python's clarity, while carving out its own identity with a concise, readable syntax that gets out of your way.

This guide walks you through the language from scratch — no prior Kiwi experience required. By the end, you'll be writing real programs with confidence.

---

## Hello, World

Every language starts here. Save this to a file called `hello.kiwi` and run it with `kiwi hello.kiwi`:

```kiwi
println "Hello, World!"
```

That's it. No boilerplate, no imports, no semicolons. Kiwi programs are clean by default.

You can also use `print` (no newline) or `println` with parentheses — both work:

```kiwi
print "Hello, "
println "World!"

println("This works too.")
```

---

## Variables

Variables in Kiwi need no declaration keyword — just assign and go:

```kiwi
name = "Kiwi"
version = 1
pi = 3.14159
active = true
nothing = null
```

Kiwi is dynamically typed, so a variable can hold any value at any time. If you want to be explicit, you can use `var` with optional type hints:

```kiwi
var greeting = "Hello"
var count: integer = 0
var label: string
```

### Multiple Assignment

You can unpack multiple values at once:

```kiwi
a, b, c = 10, 20, 30
println "${a}, ${b}, ${c}"   # 10, 20, 30
```

---

## Types

Kiwi has a small, practical set of built-in types:

| Type       | Example                        |
|------------|--------------------------------|
| `integer`  | `42`, `-7`                     |
| `float`    | `3.14`, `-0.5`                 |
| `boolean`  | `true`, `false`                |
| `string`   | `"hello"`, `'raw'`             |
| `list`     | `[1, 2, 3]`                    |
| `hashmap`  | `{name: "Alice", age: 30}`     |
| `none`     | `null`                         |

You can check a value's type at runtime using `std::type()`:

```kiwi
println std::type(42)       # integer
println std::type("hello")  # string
println std::type([1, 2])   # list
println std::type({})       # hashmap
```

---

## Strings

Strings in Kiwi come in two flavors. **Double-quoted** strings support interpolation — you can embed any expression inside `${}`:

```kiwi
name = "world"
println "Hello, ${name}!"                    # Hello, world!
println "2 + 2 = ${2 + 2}"                   # 2 + 2 = 4
println "Name is ${name.size()} chars long"  # Name is 5 chars long
```

**Single-quoted** strings are raw — no interpolation, useful for regex patterns or paths:

```kiwi
pattern = '\b[A-Z][a-z]+\b'
path = 'C:\Users\you\documents'
```

### String Operations

Strings have a rich set of methods:

```kiwi
s = "  Hello, Kiwi!  "

println s.trim()           # "Hello, Kiwi!"
println s.uppercase()      # "  HELLO, KIWI!  "
println s.lowercase()      # "  hello, kiwi!  "
println s.contains("Kiwi") # true
println s.replace("Kiwi", "World")  # "  Hello, World!  "
println s.split(", ")      # ["  Hello", "Kiwi!  "]
println s.size()           # 16
```

Strings can be repeated with `*`:

```kiwi
println "-" * 40       # ----------------------------------------
println "ha" * 3       # hahaha
```

---

## Numbers

Arithmetic works as you'd expect, with a few pleasant additions:

```kiwi
println 10 + 3    # 13
println 10 - 3    # 7
println 10 * 3    # 30
println 10 / 3    # 3.333...
println 10 % 3    # 1  (remainder)
println 10 ** 3   # 1000  (exponentiation)
```

Numbers have useful built-in methods:

```kiwi
n = -42
pi = 3.14
println n.abs()       # 42
println n.to_float()  # -42.0
println pi.floor()  # 3
println pi.ceil()   # 4
println pi.round()  # 3
```

Null-coalescing with `??` is handy when a value might be `null`:

```kiwi
value = null
result = value ?? "default"
println result   # default
```

---

## Making Decisions

### if / elsif / else

```kiwi
score = 85

if score >= 90
  println "A"
elsif score >= 80
  println "B"
elsif score >= 70
  println "C"
else
  println "Keep trying"
end
```

### Ternary Expressions

For simple conditions, the ternary operator is concise:

```kiwi
age = 20
status = age >= 18 ? "adult" : "minor"
println status   # adult
```

### The `when` Guard

Kiwi lets you attach conditions to `return`, `throw`, `next`, and `break` statements using `when`. This keeps guards clean and readable:

```kiwi
fn safe_divide(a, b)
  throw "Cannot divide by zero" when b == 0
  return a / b
end
```

### case Expressions

`case` in Kiwi is an expression — it returns a value:

```kiwi
hour = 14

time_of_day = case
  when hour < 12  "morning"
  when hour < 17  "afternoon"
  when hour < 21  "evening"
  else            "night"
end

println "Good ${time_of_day}!"   # Good afternoon!
```

You can also match against a specific value:

```kiwi
day = "Monday"

mood = case day
  when "Monday"               "Here we go again."
  when "Friday"               "Almost there!"
  when "Saturday", "Sunday"   "Enjoy the weekend."
  else                        "Just another day."
end

println mood
```

---

## Loops

### `for` Loops

The `for` loop is your everyday workhorse. It iterates over anything — lists, ranges, hashmaps:

```kiwi
fruits = ["kiwi", "mango", "lime"]

for fruit in fruits do
  println fruit
end
```

Need the index too?

```kiwi
for fruit, i in fruits do
  println "${i + 1}. ${fruit}"
end
# 1. kiwi
# 2. mango
# 3. lime
```

Ranges make numeric iteration clean:

```kiwi
for i in [1 to 5] do
  print "${i} "
end
# 1 2 3 4 5
```

Use `next when` to skip iterations and `break when` to exit early — without nested ifs:

```kiwi
for n in [1 to 20] do
  next when n % 2 == 0   # skip even numbers
  break when n > 10      # stop after 10
  println n
end
```

### `while` Loops

```kiwi
count = 1
while count <= 5 do
  println count
  count += 1
end
```

### `repeat` Loops

When you just need something done N times:

```kiwi
repeat 3 do
  println "Kiwi!"
end
```

With an iterator variable (starts at 1):

```kiwi
repeat 5 as i do
  println "Step ${i}"
end
```

---

## Collections

### Lists

Lists are ordered, mutable, and can hold mixed types:

```kiwi
colors = ["red", "green", "blue"]

println colors[0]        # red
println colors.size()    # 3

colors.push("yellow")
colors.remove("green")
println colors           # ["red", "blue", "yellow"]
```

Ranges give you lists with a single expression:

```kiwi
nums = [1 to 10]         # [1, 2, 3, 4, 5, 6, 7, 8, 9, 10]
countdown = [5 to 1]     # [5, 4, 3, 2, 1]
```

Lists have expressive built-in methods:

```kiwi
nums = [1 to 10]

evens = nums.filter(do (n) => n % 2 == 0)  # [2, 4, 6, 8, 10]
doubled = evens.map(do (n) => n * 2)       # [4, 8, 12, 16, 20]
total = doubled.sum()                      # 60

println total
```

You can chain these to build readable pipelines:

```kiwi
result = [1 to 20]
  .filter(do (n) => n % 3 == 0)
  .map(do (n) => n ** 2)
  .sum()

println result   # 735
```

### Hashmaps

Hashmaps store key-value pairs:

```kiwi
person = {
  name: "Alice",
  age: 28,
  city: "Portland"
}

println person["name"]    # Alice
println person.age        # 28 — dot notation works for simple keys

person["email"] = "alice@example.com"
person.remove("city")

println person.keys()     # ["name", "age", "email"]
```

Check for a key before accessing it:

```kiwi
if person.has_key("email")
  println "Email: ${person["email"]}"
end

# Or use get() with a default
println person.get("phone", "N/A")   # N/A
```

Iterate over a hashmap's key-value pairs:

```kiwi
for key, value in person do
  println "${key}: ${value}"
end
```

---

## Functions

Functions are defined with `fn` and end with `end`:

```kiwi
fn greet(name)
  println "Hello, ${name}!"
end

greet("Kiwi")    # Hello, Kiwi!
```

### Default Parameters

```kiwi
fn greet(name = "stranger")
  println "Hello, ${name}!"
end

greet()          # Hello, stranger!
greet("Alice")   # Hello, Alice!
```

### Return Values

The last expression in a function is its implicit return value — no `return` keyword needed (though you can use it):

```kiwi
fn add(a, b)
  a + b
end

fn square(n)
  return n * n
end

println add(3, 4)     # 7
println square(5)     # 25
```

### Named Arguments

When a function has many parameters, named arguments make call sites self-documenting:

```kiwi
fn create_user(name, age, role = "member", active = true)
  { name: name, age: age, role: role, active: active }
end

user = create_user("Bob", age: 30, role: "admin")
println user
```

### Type Hints

Type hints are optional, but they document intent and catch mismatches at runtime:

```kiwi
fn add(a: integer, b: integer): integer
  a + b
end

fn describe(val: integer|string): string
  "The value is: ${val}"
end
```

---

## Lambdas

Functions are first-class values in Kiwi. You can assign them to variables, pass them around, and return them from other functions.

The `do ... end` syntax creates a lambda:

```kiwi
square = do (n) => n * n
println square(5)   # 25
```

For multi-line lambdas, drop the arrow:

```kiwi
describe = do (n)
  if n > 0
    "positive"
  elsif n < 0
    "negative"
  else
    "zero"
  end
end

println describe(-3)    # negative
println describe(0)     # zero
```

You can also create and immediately invoke a lambda:

```kiwi
result = (do (x, y) => x * y)(6, 7)
println result   # 42
```

### Passing Lambdas to Functions

This is where Kiwi gets expressive. Built-in collection methods accept lambdas, enabling concise data transformations:

```kiwi
words = ["hello", "world", "kiwi", "lang"]

long_words = words.filter(do (w) => w.size() > 4)
upper_words = long_words.map(do (w) => w.uppercase())

println upper_words    # ["HELLO", "WORLD"]
```

### `with` Syntax

For lambdas that close over named parameters, the `with` keyword reads naturally:

```kiwi
multiply = with (a, b) do a * b end

println multiply(6, 7)   # 42
```

---

## Error Handling

Kiwi uses `try / catch / finally` for structured error handling:

```kiwi
try
  result = 10 / 0
catch (err)
  println "Something went wrong: ${err}"
finally
  println "This always runs."
end
```

You can throw your own errors as strings or as structured hashmaps:

```kiwi
fn divide(a, b)
  throw "Division by zero" when b == 0
  a / b
end

try
  println divide(10, 0)
catch (err)
  println "Caught: ${err}"
end
```

For richer error information, throw a hashmap and destructure it in the catch:

```kiwi
fn validate_age(age)
  throw { "error": "ValidationError", "message": "Age must be positive" } when age < 0
  age
end

try
  validate_age(-5)
catch (err, msg)
  println "${err}: ${msg}"   # ValidationError: Age must be positive
end
```

---

## Structs

Kiwi supports object-oriented programming through structs. A struct bundles data and behavior together:

```kiwi
struct Counter
  fn new(start = 0)
    @value = start
  end

  fn increment()
    @value += 1
  end

  fn reset()
    @value = 0
  end

  fn value()
    @value
  end

  fn to_string()
    "Counter(${@value})"
  end
end

c = Counter.new(10)
c.increment()
c.increment()
println c.value()      # 12
println c.to_string()  # Counter(12)
```

Instance variables use the `@` prefix. The `new` method acts as the constructor.

### Inheritance

Structs can inherit from other structs using `<`:

```kiwi
struct Animal
  fn new(name)
    @name = name
  end

  fn speak()
    println "${@name} makes a sound."
  end
end

struct Dog < Animal
  fn speak()
    println "${@name} barks!"
  end

  fn fetch(item)
    println "${@name} fetches the ${item}."
  end
end

d = Dog.new("Rex")
d.speak()          # Rex barks!
d.fetch("ball")    # Rex fetches the ball.
```

### Static Methods

Methods marked `static` belong to the struct itself, not to instances:

```kiwi
struct MathUtils
  static fn clamp(value, min_val, max_val)
    [min_val, [value, max_val].min()].max()
  end
end

println MathUtils.clamp(150, 0, 100)   # 100
println MathUtils.clamp(-5, 0, 100)    # 0
```

---

## A Taste of the Standard Library

Kiwi ships with a rich standard library. Here's a quick tour of what's available.

### File I/O

```kiwi
# Reading and writing files
content = fio::read("notes.txt")
fio::writeln("output.txt", ["line one", "line two"])

# Listing files
kiwi_files = fio::glob("./", ["./**/*.kiwi"])
for f in kiwi_files do
  println fio::filename(f)
end
```

### HTTP Requests

```kiwi
import "http"

response = http::get("https://httpbin.org/json")
if response.ok
  println response.body
end
```

### Math

```kiwi
import "math"

println math::sqrt(144)          # 12.0
println math::pi                 # 3.141592653589793
println math::log(math::e)       # 1.0
println math::random(1, 100)     # random integer between 1 and 100
```

### String Utilities

```kiwi
import "string"

println string::titleize("the quick brown fox")   # The Quick Brown Fox
println string::slug("Hello, World!")             # hello-world
println string::padstart("42", 6, "0")            # 000042
println string::base64encode("kiwi")              # a2l3aQ==
```

### CSV Parsing

```kiwi
import "csv"

data = csv::parse(fio::read("data.csv"))
for row in data do
  println row
end
```

---

## Putting It All Together

Here's a small but complete program that demonstrates how Kiwi's features work together. It reads a list of words, analyzes them, and prints a summary:

```kiwi
import "string"

fn analyze_words(words)
  result = {
    total: words.size(),
    longest: "",
    shortest: "",
    avg_length: 0.0,
    by_length: {}
  }

  for word in words do
    len = word.size()

    if result.longest.empty() || len > result.longest.size()
      result.longest = word
    end

    if result.shortest.empty() || len < result.shortest.size()
      result.shortest = word
    end

    count = result.by_length.get(len, 0) + 1
    result.by_length.set(len, count)
  end

  total_chars = words.map(do (w) => w.size()).sum()
  result.avg_length = total_chars.to_float() / result.total

  result
end

words = ["kiwi", "mango", "strawberry", "fig", "passionfruit", "lime", "grape"]

stats = analyze_words(words)

println "Word Analysis"
println "-" * 30
println "Total words:    ${stats.total}"
println "Longest:        ${stats.longest}"
println "Shortest:       ${stats.shortest}"
println "Average length: ${math::round(stats.avg_length, 2)}"
println ""
println "Words by length:"

for len in stats.by_length.keys().sort() do
  count = stats.by_length[len]
  bar = "#" * count
  println "  ${string::padstart(len, 2)} chars: ${bar} (${count})"
end
```

---

## What's Next

You've covered the core of Kiwi. Here are some directions to explore from here:

- **[Functions](functions.md)** — closures, recursion, variadic args
- **[Lambdas](lambdas.md)** — deeper functional programming patterns
- **[Error Handling](error_handling.md)** — structured error strategies
- **[Events](events.md)** — the built-in event bus system
- **[Standard Library](lib/README.md)** — the full library reference

The best way to learn is to write programs. Start small, experiment freely, and let the language's expressiveness guide you.

Welcome to Kiwi. 🥝
