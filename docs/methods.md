# Methods

Use the `def` keyword to define a method.

```ruby
def greet(@name)
    println "Hello, ${@name}!"
end

greet("world")
```

Use the `return` keyword to return a value from a method.

```ruby
def get_greeting(@name)
    return "Hello, ${@name}"
end

@greeting = get_greeting("World!")

println @greeting
```

### Scope

You can access all global variables from within a method.

```ruby
@counter = 0

def uptick()
  @counter += 1
end

@i = 1
while @i <= 5 do
  uptick()
  @i += 1
end

# 5
println @counter
```