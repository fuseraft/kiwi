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