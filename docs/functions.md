# Functions and Methods

Use the `fn` keyword to define a function.

Use the `def` keyword to define a method (a function of a [`class`](classes.md)). 

The distinction between a function and a method in Kiwi is purely semantic.

```kiwi
def greet(name = "Kiwi")
  println("Hello, ${name}!")
end

greet("world") # prints: Hello, world!
greet()        # prints: Hello, Kiwi!
```

### Return Value

Use the `return` keyword to return a value from a method, or to exit a method early.

```kiwi
def get_greeting(name)
  return "Hello, ${name}"
end

greeting = get_greeting("World!")

println(greeting)
```

### Optional Parameters

```kiwi
def say(msg = "Hello, World!")
  println(msg)
end

say()       # prints: Hello, World!
say("Hey!") # prints: Hey!

def configure(data, config = {})
  for key in config.keys() do
    data[key] = config[key]
  end

  return data
end

data = configure({ "name": "Scott" })
println(data) # prints: {"name": "Scott"}

data = configure({ "name": "Scott" }, { "favorite_os": "Fedora" })
println(data) # prints: {"name": "Scott", "favorite_os": "Fedora"}
```

### Scope

You can access all global variables from within a method.

```kiwi
counter = 0

def uptick()
  counter += 1
end

i = 1
while i <= 5 do
  uptick()
  i += 1
end

# 5
println(counter)
```