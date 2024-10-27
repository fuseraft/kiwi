# Structs

Structs in Kiwi provide a way to bundle data and functionality together.

### Defining Structs

To define a struct in Kiwi, use the `struct` keyword followed by the struct name and a block of code defining its properties and methods.

Each concrete struct should define a constructor method, called `new`.

```kiwi
struct MyStruct
  fn new(name)
    # Use the `@` symbol to declare an instance variable.
    @name = name 
  end

  fn say_hello()
    println("Hello, ${name}!")
  end
end
```

### Creating Instances

To create an instance of a struct, use the `.new()` method followed by any arguments the constructor accepts.

```kiwi
my_object = MyStruct.new("Kiwi")
my_object.say_hello()  # prints: Hello, Kiwi!
```

### Inheritance

Kiwi supports single inheritance. Use the `<` symbol to specify the parent struct.

```kiwi
struct MySubStruct < MyStruct
  fn new() end

  fn say_goodbye()
    println("Goodbye, ${name}!")
  end
end
```

### Method Definition

Methods are defined using the `fn` keyword, followed by the method name and any parameters. Use `@` to access the current instance.

```kiwi
struct MyStruct
  fn new() end

  fn my_method(param)
    # Method code here
  end
end
```

### Overriding `to_string()`

The `override` keyword is not required to override `to_string()`.

```kiwi
struct KiwiStruct
  fn new() end

  fn to_string()
    return "I am a Kiwi struct"
  end
end

instance = KiwiStruct.new()
string_repr = instance.to_string()
println(instance)    # prints: I am a Kiwi struct
println(string_repr) # prints: I am a Kiwi struct
```

### Static Method Definition

Methods declared as static can be invoked directly through the struct and cannot be invoked through an instance.

```kiwi
struct MyStruct
  fn new() end
  static fn static_method()
    println("I can be invoked without an instance!")
  end
end

MyStruct.static_method() # prints: I can be invoked without an instance!
```

### Access Control

Kiwi supports `private` methods that cannot be called outside the struct definition.

```kiwi
struct MyStruct
  fn new() end

  private fn my_private_method()
    # Private method code here
  end
end
```

All instance variables are private by default. You must explicitly implement accessors and modifiers to access them outside of the struct.

```kiwi
struct MyStruct
  fn new(name)
    @name = name
  end
end

inst = MyStruct.new("kiwi")
```
