# Console I/O

Kiwi offers the following builtins to work with console I/O:
- [`input(msg)`](#input): Read input from the console.
- [`print`](#print): Print text to the console.
- [`println`](#println): Print a line of text to the console.

For more advanced console I/O, please see [console](lib/console.md).

For file I/O, please see [fio](lib/fio.md).

### `input()`

To read input from the console, use the `input()` builtin.

```kiwi
print("Enter your favorite fruit: ")
fruit = input()
# fruit contains "kiwi"

println("Your favorite fruit is ${fruit}")
# prints: Your favorite fruit is kiwi
```

You can also pass a string to print as a prompt.

```kiwi
choice = ""
while !"ab".chars().contains(choice) do
  choice = input("(a)ttack or (b)lock: ")
end
```

### `print`

To print text to the console, use the `print` keyword.

```kiwi
print("Hello")
print(", World!") # prints: Hello, World!"
```

### `println`

To print a line of text to the console, use the `println` keyword.

```kiwi
println("Do you like programming?")
println("Let's build something amazing!")

# prints: 
# Do you like programming?
# Let's build something amazing!
```