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

When you run `make test`, after compiling Kiwi, the test suite will run from the [test suite](test.kiwi).

Please see the [runtime log](runtime_log.txt) for the latest results.

```ruby
# Script Performance Analyzer

import "@kiwi/fileio"
import "@kiwi/time"

@runtimes = []

# Search pattern for Kiwi scripts, excluding certain files and directories
@search_pattern = "./**/*.kiwi"
@exclusions = ["test.kiwi", "app.kiwi", "/kiwilib/", "/examples/"]

println "Searching for Kiwi scripts..."
for @file in fileio::glob(@search_pattern) do
  # Check if file is not in the exclusions list
  @exclude = false
  for @exclude_pattern in @exclusions do
    if @file.contains(@exclude_pattern)
      @exclude = true
      break
    end
  end
  
  if !@exclude
    println "Found script: ${@file}"
    
    # Record start time
    @start_time = time::ticks()
    
    # Import the script
    println "Importing and running ${@file}..."
    import @file
    
    # Record end time and calculate duration
    @end_time = time::ticks()
    @duration = time::ticksms(@end_time - @start_time)
    
    # Store runtime information
    @runtime = {"file": @file, "duration": @duration}
    @runtimes << @runtime
  end
end

# Summarize and print runtimes
@total_duration = 0
for @runtime in @runtimes do
  @file = @runtime["file"]
  @duration = @runtime["duration"]
  println "Script: ${@file} ran for ${@duration} ms."
  @total_duration += @duration
end

println "Total runtime for all scripts: ${@total_duration} ms."
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

1. **Join the Slack**: [https://kiwiprogramming.slack.com](https://kiwiprogramming.slack.com)
2. **Fork the Repository**: Fork the project to your GitHub account.
3. **Clone the Forked Repository**: Clone it to your machine.
4. **Set Up Your Environment**: Follow the "Getting Started" section.
5. **Make Changes**: Implement your features or fixes.
6. **Test Your Changes**: Ensure all tests pass.
7. **Create a Pull Request**: Submit it from your fork to the main Kiwi repository.

For more details, please read [CONTRIBUTING.md](CONTRIBUTING.md).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.