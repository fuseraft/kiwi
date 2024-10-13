# Classes

Classes in Kiwi provide a way to bundle data and functionality together.

### Defining Classes

To define a class in Kiwi, use the `class` keyword followed by the class name and a block of code defining its properties and methods.

Each concrete class should define a constructor method, called `new`.

```kiwi
class MyClass
  def new(name)
    # Use the `@` symbol to declare an instance variable.
    @name = name 
  end

  def say_hello()
    println("Hello, ${name}!")
  end
end
```

### Creating Instances

To create an instance of a class, use the `.new()` method followed by any arguments the constructor accepts.

```kiwi
my_object = MyClass.new("Kiwi")
my_object.say_hello()  # prints: Hello, Kiwi!
```

### Inheritance

Kiwi supports single inheritance. Use the `<` symbol to specify the parent class.

```kiwi
class MySubclass < MyClass
  def new() end

  def say_goodbye()
    println("Goodbye, ${name}!")
  end
end
```

### Method Definition

Methods are defined using the `def` keyword, followed by the method name and any parameters. Use `this` to access the current instance.

```kiwi
class MyClass
  def new() end

  def my_method(param)
    # Method code here
  end
end
```

### Overriding `to_string()`

The `override` keyword is not required to override `to_string()`.

```kiwi
class KiwiClass
  def new() end

  def to_string()
    return "I am a Kiwi class"
  end
end

instance = KiwiClass.new()
string_repr = instance.to_string()
println(instance)    # prints: I am a Kiwi class
println(string_repr) # prints: I am a Kiwi class
```

### Static Method Definition

Methods declared as static can be invoked directly through the class and cannot be invoked through an instance.

```kiwi
class MyClass
  def new() end
  static def static_method()
    println("I can be invoked without an instance!")
  end
end

MyClass.static_method() # prints: I can be invoked without an instance!
```

### Access Control

Kiwi supports `private` methods that cannot be called outside the class definition.

```kiwi
class MyClass
  def new() end

  private def my_private_method()
    # Private method code here
  end
end
```

You can also use `private` to declare a list of instance variable names that are to be treated as private.

```kiwi
class MyClass
  private(name) # declare instance variables as private

  def new(name)
    @name = name
  end
end

inst = MyClass.new("kiwi")
println(inst.name) # throws InvalidContextError: Cannot access private instance variable outside of object context. 
```
