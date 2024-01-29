# kiwi 🥝

A general purpose scripting language written in C++.

### Build Instructions

To build, open a terminal, navigate to the source code, and run:

```bash
make all
```

### Testing

To run the test scripts, use:
```shell
make test
```

### Test Code

Please see the `tests` directory for a list of test scripts.

### Example Code

Here's a FizzBuzz in kiwi.

```kiwi
def fizzbuzz(@n)
    @i = 1

    while @i <= @n do    
        if @i % 15 == 0
            println "FizzBuzz"
        elsif @i % 3 == 0
            println "Fizz"
        elsif @i % 5 == 0
            println "Buzz"
        else
            println "${@i}"
        endif

        @i += 1
    end
end

fizzbuzz(15)
```

### TODO

Stay tuned for updates and additions to this README.
