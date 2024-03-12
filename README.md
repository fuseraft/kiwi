# Kiwi 🥝

Kiwi is a light-weight general-purpose language designed for simplicity and versatility.

## Getting Started

To start using Kiwi, please follow the instructions below.

### Install Dependencies

#### Fedora / RHEL

```bash
sudo dnf update
sudo dnf install gcc-c++
sudo dnf install make
sudo dnf install clang-tools-extra
```

#### Ubuntu / Debian

```bash
sudo apt update
sudo apt-get install g++
sudo apt-get install make
sudo apt-get install clang-format
```

#### Installing Kiwi and Kiwi Common Library

Clone and build Kiwi.  Run `sudo make install` to run the installation script.

```bash
git clone https://github.com/fuseraft/kiwi.git
cd kiwi
sudo make install
```

#### Docker

Experiment with Kiwi in a Docker container.

```bash
sudo docker build -t kiwi-lang .
sudo docker run -it -v $(pwd):/workspace kiwi-language
```

#### Visual Studio Code Extension

You can install the [extension](https://marketplace.visualstudio.com/items?itemName=fuseraft.kiwi-lang) for syntax-highlighting in VS Code.

Launch VS Code Quick Open (`Ctrl+P`), paste the following command, and press enter.
```
ext install fuseraft.kiwi-lang
```

## Documentation

For detailed information on language features, refer to the [docs](docs/index.md).

Explore the [tests](tests/) directory for a collection of test scripts. 

To run the test suite, execute:

```shell
make test
```

#### Example: FizzBuzz

```ruby
def fizzbuzz(n)
  i = 1

  while i <= n do    
    if i % 15 == 0
      println "FizzBuzz"
    elsif i % 3 == 0
      println "Fizz"
    elsif i % 5 == 0
      println "Buzz"
    else
      println "${i}"
    end

    i += 1
  end
end

fizzbuzz(15)
```

#### Example: Sieve of Eratosthenes

```ruby
def sieve_of_eratosthenes(limit)
  isPrime = []
  for i in [0..limit] do
     isPrime << true
  end

  isPrime[0] = false
  isPrime[1] = false

  p = 2

  while p * p <= limit do
    # If p is not changed, then it is a prime
    if isPrime[p]
      # Update all multiples of p
      multiple = p * 2
      
      while multiple <= limit do
        isPrime[multiple] = false
        multiple += p
      end
    end

    p += 1
  end

  # Collect all prime numbers
  primes = []
  for i in [0..limit] do
    if isPrime[i]
      primes << i
    end
  end

  return primes
end

for prime, index in sieve_of_eratosthenes(30) do
  println "${index}:\t${prime}"
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

This project is licensed under the [MIT License](LICENSE).
