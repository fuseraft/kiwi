# Conditions

I have removed `unless` for now. I am currently refactoring condition and loop logic at the moment.

## If Statements
Each `if` statement ends with an `end`. *See the test script for a better example with nesting.*

```ruby
@a = 0, @b = 1

if @a < @b
    println "${@a} is less than ${@b}"
elsif @a > @b
    println "${@a} is greater than ${@b}"
end
```

## Switch Statements

*Under construction in new interpreter.*

```ruby
@n = 5
switch @n
    case 5
        println "five"
    default
        println "not five"
end
```