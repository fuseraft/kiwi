# kiwi 🥝

A general-purpose object-oriented scripting language influenced by Ruby.

## Getting Started

To get started with Kiwi, clone the repository and build the source:

```bash
git clone https://github.com/fuseraft/kiwi.git
cd kiwi
make all
```

## Testing

Please see the [tests](tests) directory for a list of test scripts.

To run the test suite, use:

```shell
make test
```

## Test Suite

When you run `make test`, after compiling Kiwi, the test suite will run from the [test suite script](test.kiwi).

Please see the [runtime log](runtime_log.txt) for the latest results.

```ruby
##
Search for all Kiwi scripts in all subdirectories in the current directory, excluding all test.kiwi files and kiwilib.

Import each script and record the duration of each run.

Iterate through all the recorded runtimes per script and print the details of each, then print the totals at the end.

I really hope you enjoy Kiwi as much as I do! :)
##

import "@kiwi/fileio"
import "@kiwi/time"

@runtimes = []

for @file in fileio::glob("./**/*.kiwi") do
  if !(@file.contains("test.kiwi") || @file.contains("/kiwilib/"))
    @runtime = {"file": @file}

    # Import the script and record the duration.
    @start = time::ticks()
    import @file
    @stop = time::ticks()

    @runtime["duration"] = @stop - @start
    @runtimes << @runtime
  end
end

@totalRuntime = 0

# Print the runtime details and totals at the end.
for @runtime, @testno in @runtimes do
  @ticks = @runtime["duration"]
  @file = @runtime["file"]
  @duration = time::ticksms(@ticks)
  @totalRuntime += @duration

  println "Test #${@testno}, ran ${@file} for ${@duration}ms."
end

println "Ran ${@runtimes.size()} test(s) in ${@totalRuntime}ms."
```

## Example: FizzBuzz

```ruby
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
    end

    @i += 1
  end
end

fizzbuzz(15)
```

## Example: Sieve of Eratosthenes

```ruby
def sieve_of_eratosthenes(@limit)
  @isPrime = []
  for @i in [0..@limit] do
     @isPrime << true
  end

  @isPrime[0] = false
  @isPrime[1] = false

  @p = 2

  while @p * @p <= @limit do
    # If @p is not changed, then it is a prime
    if @isPrime[@p]
      # Update all multiples of @p
      @multiple = @p * 2
      
      while @multiple <= @limit do
        @isPrime[@multiple] = false
        @multiple += @p
      end
    end

    @p += 1
  end

  # Collect all prime numbers
  @primes = []
  for @i in [0..@limit] do
    if @isPrime[@i]
      @primes << @i
    end
  end

  return @primes
end

for @prime, @index in sieve_of_eratosthenes(30) do
  println "${@index}:\t${@prime}"
end
```

## Contributions

I welcome and appreciate any and all contributions to the Kiwi project! Here's how you can contribute:

1. **Fork the Repository**: Fork the project to your GitHub account.
2. **Clone the Forked Repository**: Clone it to your machine.
3. **Set Up Your Environment**: Follow the "Getting Started" section.
4. **Make Changes**: Implement your features or fixes.
5. **Test Your Changes**: Ensure all tests pass.
6. **Create a Pull Request**: Submit it from your fork to the main Kiwi repository.

For more details, please read [CONTRIBUTING.md](CONTRIBUTING.md).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.