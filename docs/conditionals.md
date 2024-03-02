# Conditionals

## If Statements
Each `if` statement ends with an `end`. *See the test script for a better example with nesting.*

```ruby
@a = 0, @b = 1

if @a < @b
  println "${@a} is less than ${@b}"
elsif @a > @b
  println "${@a} is greater than ${@b}"
else
  println "${@a} is equal to ${@b}"
end
```