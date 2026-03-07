# Operator Overloading

Structs in Kiwi can define custom behavior for built-in operators by declaring methods whose names are operator symbols.

---

## Defining an Overload

Inside a struct, use `fn` followed by the operator symbol as the method name. The method receives the right-hand operand as its single parameter; the left-hand operand is the instance itself (`@`).

```kiwi
struct Vec2
  fn new(x, y)
    @x = x
    @y = y
  end

  fn +(other: Vec2): Vec2
    Vec2.new(@x + other.x, @y + other.y)
  end

  fn *(scalar): Vec2
    Vec2.new(@x * scalar, @y * scalar)
  end

  fn ==(other: Vec2): boolean
    @x == other.x && @y == other.y
  end

  fn to_string(): string
    "(${@x}, ${@y})"
  end
end

a = Vec2.new(1, 2)
b = Vec2.new(3, 4)

println a + b       # (4, 6)
println a * 3       # (3, 6)
println a == b      # false
println a == a      # true
```

---

## Supported Operators

Any binary operator can be overloaded. The overload is triggered when the **left operand** is an instance of the struct.

| **Category** | **Operators** |
| ------------ | ------------- |
| Arithmetic | `+` `-` `*` `/` `%` `**` |
| Comparison | `==` `!=` `<` `<=` `>` `>=` |
| Logical | `&&` `\|\|` |
| Bitwise | `&` `\|` `^` `<<` `>>` `>>>` |

> Unary operators (`!`, `~`, `-` as negation) and assignment operators (`=`, `+=`, etc.) cannot be overloaded.

---

## Practical Example — A `Number` Wrapper

```kiwi
struct Number
  fn new(val)
    @val = val
  end

  fn +(other): Number
    Number.new(@val + (typeof(other) == "Number" ? other.val : other))
  end

  fn -(other): Number
    Number.new(@val - (typeof(other) == "Number" ? other.val : other))
  end

  fn ==(other): boolean
    @val == (typeof(other) == "Number" ? other.val : other)
  end

  fn to_string(): string
    "${@val}"
  end
end

n = Number.new(10)
println n + Number.new(5)  # 15
println n - 3              # 7
println n == 10            # true
```

---

## Inheritance

Operator overloads are inherited. A derived struct that does not define its own overload will use the one from its base struct.

```kiwi
struct Base
  fn new(v)
    @v = v
  end

  fn +(other): Base
    Base.new(@v + other.v)
  end

  fn to_string(): string
    "${@v}"
  end
end

struct Derived < Base
  # inherits + from Base
end

a = Derived.new(3)
b = Derived.new(7)
println a + b   # 10
```

---

## Limitations

- **Structs only** — operator overloading is not supported at the global scope.
- **Left operand** — the overload is dispatched on the type of the *left* operand. If the left operand is not an instance of a struct with a matching overload, normal built-in semantics apply.
- **Single parameter** — the overload method always takes exactly one parameter (the right-hand operand).
