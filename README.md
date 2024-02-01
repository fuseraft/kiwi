# kiwi 🥝

A general-purpose object-oriented scripting language influenced by Ruby.

## Getting Started

To get started with Kiwi, clone the repository and build the source:

```bash
git clone https://github.com/fuseraft/kiwi.git
cd kiwi
make all
```

## Build Instructions

To build Kiwi, open a terminal, navigate to the source code, and run:

```
make all
```

## Testing

To run the test scripts, use:

```shell
make test
```

To test the Kiwi Common Library, use:

```shell
make testlib
```

## Test Code

Please see the [tests](tests) directory for a list of test scripts.

## Example: FizzBuzz in Kiwi

Here's an example of FizzBuzz implemented in Kiwi:

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

## Contributions

We welcome contributions to the Kiwi project! Here's how you can contribute:

1. **Fork the Repository**: Fork the project to your GitHub account.
2. **Clone the Forked Repository**: Clone it to your machine.
3. **Set Up Your Environment**: Follow the "Getting Started" section.
4. **Make Changes**: Implement your features or fixes.
5. **Test Your Changes**: Ensure all tests pass.
6. **Create a Pull Request**: Submit it from your fork to the main Kiwi repository.

For more details, please read [CONTRIBUTING.md](CONTRIBUTING.md).

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.