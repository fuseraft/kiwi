# Kiwi Classes Documentation

Classes in Kiwi provide a mechanism for bundling data and functionality together. Using classes, you can create new objects, encapsulate data, and define methods for interacting with that data.

## Defining Classes

To define a class in Kiwi, use the `class` keyword followed by the class name and a block of code defining its properties and methods.

```kiwi
class MyClass
  def initialize(@name)
    # Constructor code here

    # Use the `this` keyword to declare an instance variable.
    this.@name = @name 
  end

  def say_hello()
    println "Hello, ${@name}!"
  end
end
```

## Creating Instances

To create an instance of a class, use the `.new()` method followed by any arguments the constructor accepts.

```kiwi
@my_object = MyClass.new("Kiwi")
@my_object.say_hello()  # Output: Hello, Kiwi!
```

## Inheritance

Kiwi supports single inheritance. Use the `<` symbol to specify the parent class.

```kiwi
class MySubclass < MyClass
  def say_goodbye()
    println "Goodbye, ${@name}!"
  end
end
```

## Method Definition

Methods are defined using the `def` keyword, followed by the method name and any parameters. Use `this` to access the current instance.

```ruby
class MyClass
  def my_method(@param)
    # Method code here
  end
end
```

## Static Method Definition

Methods declared as static can be invoked directly through the class and cannot be invoked through an instance.

```ruby
class MyClass
  static def static_method()
    println "I can be invoked without an instance!"
  end
end

MyClass.static_method() # prints: I can be invoked without an instance!
```

## Access Control

Kiwi supports `private` methods that cannot be called outside the class definition.

```ruby
class MyClass
  private def my_private_method()
    # Private method code here
  end
end
```