# Abstract Classes

Abstract classes in Kiwi serve as a way to define classes that cannot be instantiated on their own but can be used as a base for other classes. These classes are intended to outline a set of methods and properties that derived classes must implement or inherit.

## Defining Abstract Classes

To define an abstract class, use the `abstract class` keywords followed by the class name. Abstract methods within these classes are declared using the `abstract def` keywords.

```kiwi
abstract class Shape
  abstract def area()
  abstract def perimeter()
end
```

## Implementing Abstract Classes

When you inherit from an abstract class, you must provide implementations for all the abstract methods defined in the base class.

Use the `override` keyword to override the abstract method with an implementation.

```kiwi
class Circle < Shape
  def initialize(radius)
    @radius = radius
  end

  override def area()
    return 3.14159 * @radius * @radius
  end

  override def perimeter()
    return 2 * 3.14159 * @radius
  end
end
```

## Instantiation

Abstract classes cannot be instantiated directly. An attempt to create an instance of an abstract class will result in an error. Instead, you must instantiate a subclass that implements all abstract methods of the base class.

```kiwi
# This will result in an error:
# shape = Shape.new()

# Correct way to instantiate:
circle = Circle.new(5)
println(circle.area())       # Output: Area of the circle.
println(circle.perimeter())  # Output: Perimeter of the circle.
```

## Notes

- Abstract classes are a foundational concept for enforcing a contract for subclasses, ensuring that they implement specific methods.
- While you cannot create instances of abstract classes, they are crucial for designing a clear and consistent class hierarchy in object-oriented programming in Kiwi.
