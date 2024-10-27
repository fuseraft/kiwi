# Abstract Structs

Abstract structs in Kiwi serve as a way to define structs that cannot be instantiated on their own but can be used as a base for other structs. These structs are intended to outline a set of methods and properties that derived structs must implement or inherit.

## Defining Abstract Structs

To define an abstract struct, use the `abstract struct` keywords followed by the struct name. Abstract methods within these structs are declared using the `abstract def` keywords.

```kiwi
abstract struct Shape
  abstract fn area()
  abstract fn perimeter()
end
```

## Implementing Abstract Structs

When you inherit from an abstract struct, you must provide implementations for all the abstract methods defined in the base struct.

Use the `override` keyword to override the abstract method with an implementation.

```kiwi
struct Circle < Shape
  fn new(radius)
    @radius = radius
  end

  override fn area()
    return 3.14159 * @radius * @radius
  end

  override fn perimeter()
    return 2 * 3.14159 * @radius
  end
end
```

## Instantiation

Abstract structs cannot be instantiated directly. An attempt to create an instance of an abstract struct will result in an error. Instead, you must instantiate a child struct that implements all abstract methods of the base struct.

```kiwi
# This will result in an error:
# shape = Shape.new()

# Correct way to instantiate:
circle = Circle.new(5)
println(circle.area())       # Output: Area of the circle.
println(circle.perimeter())  # Output: Perimeter of the circle.
```

## Notes

- Abstract structs are a foundational concept for enforcing a contract for child structs, ensuring that they implement specific methods.
- While you cannot create instances of abstract structs, they are crucial for designing a clear and consistent struct hierarchy in object-oriented programming in Kiwi.
