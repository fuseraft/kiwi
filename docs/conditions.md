# Conditions

I have removed `unless` for now. I am currently refactoring condition and loop logic at the moment.

## If Statements
Each `if` statement ends with an `endif`.

```
@a = 0
@b = 1

if @a < @b
    say "\{@a} is less than \{@b}"
elsif @a > @b
    say "\{@a} is greater than \{@b}"
endif
```

## Switch Statements

```
@n = 5
switch @n
    case 5
        println "five"
    default
        println "not five"
end
```