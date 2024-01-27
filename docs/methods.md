# Methods

*Under construction in new interpreter.*

Use the `def` keyword to define a method.

```
def greet(name)
    println "Hello, ${name}!"
end

greet("world")
```

The last assignment is the return value.

```
def add_three(first, second, third)
    @ret = (@first + @second + @third)
end

@ans = add_three(5, 4, 3)

if @ans == 12
  println "5 + 4 + 3 = 12"
endif
```

### Scope

You can access all global variables from within a method.

```kiwi
@counter = 0

def uptick
  @counter += 1
end

for 1 to 5
  uptick
endfor

# 5
println @counter
```