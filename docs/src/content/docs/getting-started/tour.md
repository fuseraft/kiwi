---
title: A Tour of Kiwi
description: The whole language in one page, with links to the details.
---

This page walks through Kiwi from top to bottom. Each section links to the page that covers the topic in full. You can paste any example into a file and run it with `kiwi file.kiwi`, or try it line by line in the [REPL](/kiwi/tools/repl/).

## Printing

`println` prints a value followed by a newline; `print` leaves the newline off. Parentheses are optional.

```kiwi
print "Hello, "
println "World!"
println("Parentheses work too.")
```

## Variables

Assign to a name to create a variable. There is no declaration keyword, and a variable can hold any type.

```kiwi
name = "Kiwi"
version = 1
pi = 3.14159
active = true
nothing = null

a, b, c = 10, 20, 30
println "${a}, ${b}, ${c}"   # 10, 20, 30
```

`const` declares a value that can't be reassigned. Constant names are upper case.

```kiwi
const MAX_RETRIES = 3
```

See [Variables](/kiwi/language/variables/).

## Types

| Type | Examples |
| :-- | :-- |
| `integer` | `42`, `-7`, `0xFF`, `0b1010` |
| `float` | `3.14`, `-0.5`, `1.5e3` |
| `boolean` | `true`, `false` |
| `string` | `"hello"`, `'raw'` |
| `list` | `[1, 2, 3]` |
| `hashmap` | `{name: "Alice", age: 30}` |
| `null` | `null` |

`typeof` names a value's type:

```kiwi
println typeof(42)        # integer
println typeof("hello")   # string
println typeof([1, 2])    # list
println typeof({})        # hashmap
```

Kiwi also has `date`, `bytes`, `lambda`, `generator` and struct instances. See [Types](/kiwi/language/types/).

## Strings

Strings interpolate any expression inside `${...}`. Single-quoted strings are raw: backslashes are kept as written, which suits regular expressions and Windows paths.

```kiwi
name = "world"
println "Hello, ${name}!"            # Hello, world!
println "2 + 2 = ${2 + 2}"           # 2 + 2 = 4
println "${name.size()} characters"  # 5 characters

pattern = '\d+'
println "a1b22".scan(pattern)        # ["1", "22"]
```

Strings come with a set of built-in methods:

```kiwi
s = "  Hello, Kiwi!  "
println s.trim()                     # Hello, Kiwi!
println s.uppercase()                #   HELLO, KIWI!
println s.contains("Kiwi")           # true
println s.trim().split(", ")         # ["Hello", "Kiwi!"]
println "-" * 10                     # ----------
```

See [Strings](/kiwi/language/strings/).

## Numbers

```kiwi
println 10 + 3    # 13
println 10 - 3    # 7
println 10 * 3    # 30
println 10 / 4    # 2.5   (/ always produces a float)
println 10 // 4   # 2     (floor division)
println 10 % 3    # 1
println 2 ** 10   # 1024
```

`??` picks the right-hand side when the left is `null`:

```kiwi
value = null
println value ?? "default"   # default
```

See [Operators](/kiwi/language/operators/).

## Making decisions

```kiwi
score = 85

if score >= 90
  println "A"
elsif score >= 80
  println "B"
else
  println "Keep going"
end
```

`case` is an expression. It can test conditions or compare one value against several:

```kiwi
hour = 14

part_of_day = case
  when hour < 12 "morning"
  when hour < 17 "afternoon"
  else           "evening"
end
println "Good ${part_of_day}!"   # Good afternoon!

day = "Saturday"
mood = case day
  when "Monday"             "Here we go again."
  when "Saturday", "Sunday" "Weekend!"
  else                      "Just another day."
end
println mood                     # Weekend!
```

A `when` guard makes `return`, `throw`, `break`, `next` and `exit` conditional, which keeps early exits on one line:

```kiwi
fn safe_divide(a, b)
  throw "Cannot divide by zero" when b == 0
  return a / b
end

println safe_divide(9, 3)   # 3
```

See [Control Flow](/kiwi/language/control-flow/).

## Loops

```kiwi
fruits = ["kiwi", "mango", "lime"]

for fruit in fruits do
  println fruit
end

for fruit, i in fruits do
  println "${i + 1}. ${fruit}"
end

for n in [1 to 20] do
  next when n % 2 == 0
  break when n > 7
  print "${n} "
end
println ""                     # 1 3 5 7

repeat 3 as i do
  println "Step ${i}"          # Step 1, Step 2, Step 3
end

count = 0
while count < 3 do
  count += 1
end
```

See [Loops](/kiwi/language/loops/).

## Lists

Lists are ordered and can hold values of any type. Indexing starts at 0.

```kiwi
colors = ["red", "green", "blue"]
println colors[0]          # red
println colors.size()      # 3

colors.push("yellow")
colors.remove("green")
println colors             # ["red", "blue", "yellow"]
println colors[0:2]        # ["red", "blue"]
```

`[a to b]` builds a range, and list methods take lambdas:

```kiwi
total = [1 to 20]
  .filter(do (n) => n % 3 == 0)
  .map(do (n) => n * n)
  .sum()

println total   # 819
```

See [Lists](/kiwi/language/lists/).

## Hashmaps

```kiwi
person = {
  name: "Alice",
  age: 28,
  city: "Portland"
}

println person["name"]       # Alice
println person.age           # 28

person["email"] = "alice@example.com"
person.remove("city")
println person.keys()        # ["name", "age", "email"]
println person.get("phone", "n/a")   # n/a

for key, value in person do
  println "${key}: ${value}"
end
```

See [Hashmaps](/kiwi/language/hashmaps/).

## Functions

A function returns the value of its last expression, so `return` is only needed to leave early.

```kiwi
fn add(a, b)
  a + b
end

fn greet(name = "stranger", greeting = "Hello")
  "${greeting}, ${name}!"
end

println add(3, 4)                    # 7
println greet()                      # Hello, stranger!
println greet("Bob", greeting: "Hi") # Hi, Bob!
```

Functions can take a variable number of arguments, and `*list` spreads a list into arguments:

```kiwi
fn sum(*numbers)
  numbers.sum()
end

println sum(1, 2, 3)     # 6
println sum(*[4, 5, 6])  # 15
```

See [Functions](/kiwi/language/functions/).

## Lambdas

`do (params) => expression` creates a lambda. For more than one expression, drop the arrow and close with `end`.

```kiwi
square = do (n) => n * n
println square(5)   # 25

describe = do (n)
  if n > 0
    "positive"
  else
    "not positive"
  end
end
println describe(-3)   # not positive
```

Lambdas close over the variables around them:

```kiwi
fn make_counter()
  count = 0
  do ()
    count += 1
    count
  end
end

next_id = make_counter()
next_id()
println next_id()   # 2
```

See [Lambdas](/kiwi/language/lambdas/).

## Errors

```kiwi
fn divide(a, b)
  throw error::ValueError("b must not be zero") when b == 0
  a / b
end

try
  divide(1, 0)
catch e
  println "${e["error"]}: ${e["message"]}"   # ValueError: b must not be zero
finally
  println "done"
end
```

See [Error Handling](/kiwi/language/errors/).

## Structs

Structs bundle data with behavior. `@name` is an instance variable and `new` is the constructor.

```kiwi
struct Animal
  fn new(name)
    @name = name
  end

  fn speak()
    "${@name} makes a sound."
  end
end

struct Dog < Animal
  fn speak()
    "${@name} barks!"
  end
end

println Animal.new("Generic").speak()   # Generic makes a sound.
println Dog.new("Rex").speak()          # Rex barks!
```

See [Structs](/kiwi/language/structs/).

## Packages

A `package` groups functions under a name. Call them with `::`.

```kiwi
package geometry
  fn area(w, h)
    w * h
  end
end

println geometry::area(3, 4)   # 12
```

Your own files load with `import "path/to/file"`. See [Packages](/kiwi/language/packages/).

## The standard library

The standard library is loaded automatically; there is nothing to import.

```kiwi
println math::sqrt(144.0)                   # 12
println string::titleize("the quick fox")   # The Quick Fox
println json::stringify({a: 1})             # {"a":1}
println time::now().year() >= 2024          # true
```

```kiwi nocheck
content = fio::read("notes.txt")
response = http::get("https://example.com")
rows = csv::parse(fio::read("data.csv"))
```

Browse the [Standard Library](/kiwi/stdlib/) for the full list of packages.
