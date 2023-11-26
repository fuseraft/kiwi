# Conditions

I have removed `unless` for now. I am currently refactoring condition and loop logic at the moment.

You have simple `if` statements for now. Each `if` statement ends with an `endif`.

```
@a = 0
@b = 1

if @a < @b
    say "\{@a} is less than \{@b}"
elif @a > @b
    say "\{@a} is greater than \{@b}"
endif
```
