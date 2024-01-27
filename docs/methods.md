# Methods

Use the `def` keyword to define a method.

```
def greet(@name)
    println "Hello, ${@name}!"
end

greet(@name = "world")
```

The last assignment is the return value.

```
def add_three(@a, @b, @c)
    @ret = (@a + @b + @c)
end

@ans = add_three(@a = 5, @b = 4, @c = 3)

if @ans == 12
  println "Pass!"
endif
```

### Scope

You can access all global variables from within a method.

```kiwi
@counter = 0

def uptick
  @counter += 1
end

@i = 1
while @i <= 5
  uptick
  @i += 1
endfor

# 5
println @counter
```