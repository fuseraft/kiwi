# uslang

An unorthodox scripting language written in C++.

## Build Instructions

To build, open a terminal, navigate to the source code, and run:

```bash
make all
```

## Testing

To run the test scripts, use:
```shell
make test
```

## Test Code

Please see the `tests` directory for a list of test scripts.

## Example Code

Here's a FizzBuzz in USL.

```uslang
@i = 0

while @i <= 15 do
    @i += 1
    
    if @i % 15 == 0
        println "FizzBuzz"
    elsif @i % 3 == 0
        println "Fizz"
    elsif @i % 5 == 0
        println "Buzz"
    else
        println "${@i}"
    endif
end
```

## History

I began writing this scripting language as a junior in high school.

It began as a side project to build an alternative shell for Linux and evolved into what it is today.

## TODO

Stay tuned for updates and additions to this README.