# Operator Overloading

> Giving structs their own arithmetic and comparison operators.

A struct can define how arithmetic and comparison operators behave for its instances. Name a method after the operator; it receives the right-hand operand, and `@` is the left-hand one.

```kiwi
struct Vec2
  fn new(x, y)
    @x = x
    @y = y
  end

  fn +(other)
    Vec2.new(@x + other.x, @y + other.y)
  end

  fn *(scalar)
    Vec2.new(@x * scalar, @y * scalar)
  end

  fn ==(other)
    @x == other.x && @y == other.y
  end

  fn to_string()
    "(${@x}, ${@y})"
  end
end

a = Vec2.new(1, 2)
b = Vec2.new(3, 4)

println (a + b).to_string()   # (4, 6)
println (a * 3).to_string()   # (3, 6)
println a == b                # false
println a != b                # true
```

## Which operators

| Category | Operators |
| :-- | :-- |
| Arithmetic | `+`, `-`, `*`, `/`, `//`, `%`, `**` |
| Comparison | `==`, `!=`, `<`, `<=`, `>`, `>=` |
| Bitwise | `&`, `\|`, `^`, `<<`, `>>`, `>>>` |

If you define `==` but not `!=`, `!=` returns the opposite of your `==`.

Logical operators (`&&`, `||`, which short-circuit), unary operators, indexing and assignment can't be overloaded.

## Rules

- **The left operand decides.** `vec + 1` calls `Vec2`'s `+`, but `1 + vec` uses the built-in integer `+`. Put the struct on the left.
- **One parameter.** An operator method takes exactly one parameter: the right-hand operand.
- **Compound assignment follows.** `a += b` uses your `+`.
- **Overloads are inherited.** A child struct uses its parent's operators unless it defines its own.

## Accepting different right-hand types

Check the right operand's type with `typeof` to support more than one kind:

```kiwi
struct Money
  fn new(cents)
    @cents = cents
  end

  fn +(other)
    extra = typeof(other) == "Money" ? other.cents : other
    Money.new(@cents + extra)
  end

  fn <(other)
    @cents < other.cents
  end
end

total = Money.new(500) + Money.new(250) + 100
println total.cents   # 850

prices = [Money.new(300), Money.new(100), Money.new(200)]
cheapest = prices.sort(do (a, b) => a < b).first()
println cheapest.cents   # 100
```

Use `typeof(value) == "Name"` rather than `value.is_a(Name)` here: `.is_a` with a struct argument only works when `value` is itself a struct instance.

Since `println` doesn't call `to_string()`, call it explicitly when printing results, as in the first example.
