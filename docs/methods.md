# Methods

Use the `method` keyword to define a method.

```
method greet(name)
    println "Hello, \{@name}!"
end

greet("world")
```

The last assignment is the return value.

```
method add_three(first, second, third)
    @ret = (@first + @second + @third)
end

@ans = add_three(5, 4, 3)

if @ans == 12
  println "5 + 4 + 3 = 12"
endif
```