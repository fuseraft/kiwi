# Structs

> Defining types with structs: constructors, instance and static members, inheritance, abstract structs and private methods.

A struct defines a type that bundles data with the methods that work on it.

## Defining a struct

```kiwi
struct Point
  fn new(x, y)
    @x = x
    @y = y
  end

  fn distance_to(other)
    dx = other.x - @x
    dy = other.y - @y
    math::sqrt((dx * dx + dy * dy).to_float())
  end

  fn describe()
    "(${@x}, ${@y})"
  end
end

a = Point.new(0, 0)
b = Point.new(3, 4)
println a.distance_to(b)   # 5
println b.describe()       # (3, 4)
```

- `new` is the constructor. `StructName.new(...)` creates an instance and calls it. A struct without a `new` method can still be instantiated with `StructName.new()`.
- `@name` reads or writes an instance variable of the current object. Instance variables are created by assignment, usually in `new`.
- Methods call each other directly, as in `twice()` below, or through `@`, the current instance.

## Instance variables

Instance variables are visible from outside as properties, and can be assigned there too:

```kiwi
struct Account
  fn new(owner)
    @owner = owner
    @balance = 0
  end
end

acct = Account.new("Ada")
println acct.owner     # Ada
acct.balance = 50
println acct.balance   # 50
```

Prefer methods for anything with rules attached, so callers don't need to know the internals.

### Instance variables in lambdas

Inside a lambda, `@` does not currently refer to the instance, so `@name` reads as `null`. Copy what the lambda needs into a local variable first:

```kiwi
struct Pricing
  fn new(rate)
    @rate = rate
  end

  fn apply(prices)
    rate = @rate
    prices.map(do (p) => p * rate)
  end
end

println Pricing.new(2).apply([1, 5])   # [2, 10]
```

## Methods that return `@`

Returning `@` from a method allows chained calls:

```kiwi
struct Builder
  fn new()
    @parts = []
  end

  fn add(part)
    @parts.push(part)
    @
  end

  fn count()
    @parts.size()
  end

  fn twice()
    count() * 2
  end

  fn build()
    @parts.join(" ")
  end
end

b = Builder.new().add("kiwi").add("is").add("tasty")
println b.build()   # kiwi is tasty
println b.twice()   # 6
```

## Converting to a string

Define `to_string()` to give a struct a readable form, and call it explicitly. `println` and string interpolation don't call it automatically; they print the generic `<struct(...)>` form.

```kiwi
struct Money
  fn new(cents)
    @cents = cents
  end

  fn to_string()
    "$" + (@cents / 100.0).to_string("f2")
  end
end

price = Money.new(1999)
println price.to_string()          # $19.99
println "Total: ${price.to_string()}"
```

## Static members

`static fn` defines a method called on the struct itself. `static @name = value` defines a variable shared by every instance; methods reach it with `@@name`, and outside code with `StructName.name`.

```kiwi
struct Ticket
  static @issued = 0

  fn new(holder)
    @@issued += 1
    @number = @@issued
    @holder = holder
  end

  static fn count()
    @@issued
  end
end

Ticket.new("Ada")
t = Ticket.new("Grace")

println t.number         # 2
println Ticket.count()   # 2
println Ticket.issued    # 2

Ticket.issued = 0
println Ticket.count()   # 0
```

:::caution
Static variables are not currently inherited. A parent method that uses `@@name` fails when it runs on an instance of a child struct, because `@@name` is looked up on the child.
:::

## Inheritance

`struct Child < Parent` inherits every method of `Parent`. A child can override a method by defining one with the same name. Kiwi has single inheritance.

```kiwi
struct Animal
  fn new(name)
    @name = name
  end

  fn speak()
    "..."
  end

  fn introduce()
    "${@name} says ${speak()}"
  end
end

struct Dog < Animal
  fn speak()
    "Woof"
  end
end

println Dog.new("Rex").introduce()   # Rex says Woof
```

`override` before `fn` marks a method as an intentional override. It is optional and currently informational.

### Calling the parent with `super`

`super.method(...)` calls the parent struct's version of a method on the current instance. It works for constructors and for any other method, and resolves relative to the struct that defined the method being run, so chains of several levels work as expected.

```kiwi
struct Animal
  fn new(name)
    @name = name
  end

  fn speak()
    "..."
  end
end

struct Dog < Animal
  fn speak()
    "Woof"
  end
end

struct PoliceDog < Dog
  fn new(name, badge)
    super.new(name)
    @badge = badge
  end

  fn speak()
    super.speak() + "! (badge #${@badge})"
  end
end

println PoliceDog.new("Rex", 7).speak()   # Woof! (badge #7)
```

## Abstract structs

An `abstract struct` can't be instantiated; it exists to be inherited. Inside it, `abstract fn name(params) end` declares a method that every concrete child must implement. A child that misses one is rejected as soon as it is defined.

```kiwi
abstract struct Shape
  abstract fn area() end

  fn describe()
    "${typeof(@)} with area ${area()}"
  end
end

struct Square < Shape
  fn new(side)
    @side = side
  end

  fn area()
    @side * @side
  end
end

println Square.new(3).describe()   # Square with area 9
```

```kiwi
abstract struct Shape
  abstract fn area() end
end

Shape.new()   # AbstractInstantiationError: Cannot instantiate abstract struct `Shape`.
```

```kiwi
abstract struct Shape
  abstract fn area() end
end

struct Blob < Shape
end
# AbstractMethodError: Struct `Blob` must implement abstract method `area`.
```

## Private methods

`private fn` marks a method as an internal helper. By convention, call private methods only from inside the struct.

```kiwi
struct Password
  fn new(text)
    @hash = digest(text)
  end

  fn matches(text)
    digest(text) == @hash
  end

  private fn digest(text)
    crypto::sha256_hash(text)
  end
end

p = Password.new("hunter2")
println p.matches("hunter2")   # true
```

:::note
Kiwi does not currently enforce `private`; a call from outside the struct still succeeds.
:::

## Working with instances

| Method | Returns |
| :-- | :-- |
| `.is_a(Struct)` | Whether the instance is that struct or inherits from it |
| `.type()`, `typeof(obj)` | The struct's name |
| `.keys()` | The instance variable names |
| `.values()` | The instance variable values |
| `.set(name, value)` | Sets an instance variable by name |
| `.clone()` | A new instance with the same instance variables |

```kiwi
struct User
  fn new(name, role)
    @name = name
    @role = role
  end
end

u = User.new("Ada", "admin")
println u.is_a(User)    # true
println typeof(u)       # User
println u.keys()        # ["name", "role"]
println u.values()      # ["Ada", "admin"]

u.set("role", "owner")
println u.role          # owner
```

`.clone()` copies the instance variables but not the lists or hashmaps they point to: a cloned instance shares those with the original.

Two instances are `==` when they are the same struct and their instance variables are equal.

## Related pages

- [Operator Overloading](operator-overloading.md) to give structs `+`, `==` and other operators.
- [Decorators](decorators.md) work on methods too.
- [`hashable`](../lib/hashable.md) adds `to_hash()` for converting instances to hashmaps.
- [Enums](enums.md) for named sets of constants.
