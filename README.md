# Kiwi 🥝

Kiwi is a light-weight general-purpose language designed for simplicity and versatility.

## Getting Started

To start using Kiwi, please follow the instructions below.

### Docker

Experiment with Kiwi in a Docker container.

```bash
sudo docker build -t kiwi-lang .
sudo docker run -it -v $(pwd):/workspace kiwi-language
```

### Building Kiwi

To build Kiwi, use your favorite C++ compiler.

This repo uses GCC and Make.

#### Fedora / RHEL

```bash
sudo dnf update
sudo dnf install gcc-c++
sudo dnf install make
```

#### Ubuntu / Debian

```bash
sudo apt update
sudo apt-get install g++
sudo apt-get install make
```

Then build with Make.

```bash
make
```

#### Installing Kiwi and Kiwi Common Library

Clone and build Kiwi.  Run `sudo make install` to build Kiwi and run the installation script.

```bash
git clone https://github.com/fuseraft/kiwi.git
cd kiwi
sudo make install
```

#### Visual Studio Code Extension

You can install the [extension](https://marketplace.visualstudio.com/items?itemName=fuseraft.kiwi-lang) for syntax-highlighting in VS Code.

Launch VS Code Quick Open (`Ctrl+P`), paste the following command, and press enter.
```
ext install fuseraft.kiwi-lang
```

## Documentation

For detailed information on language features, refer to the [docs](docs/README.md).

Explore the [tests](tests/) directory for a collection of test scripts. 

To run the test suite, execute:

```shell
make test
```

#### Example Code

For fun, I wrote some [Project Euler examples](examples/project_euler/).

Below is a sample script that generates a temporary script and executes it.

```ruby
import "@kiwi/fileio" as fs
import "@kiwi/math" as math
import "@kiwi/sys" as sys

try
  # Look for a temporary directory. Fail fast.
  if !fs.exists(fs.tmpdir())
    println "Could not find temporary directory."
    exit 1
  end
  
  # Generate a random temporary file path.
  filename = math.random("0123456789ABCDEF", 10)
  path = fs.combine(fs.tmpdir(), "${filename}.🥝")

  # Write a little 🥝 script.
  fs.write(path, "
    ##
    This script will delete itself and let you know it was there.
    ##
    import \"@kiwi/fileio\" as fs
    fs.remove(\"${@path}\")
    println \"Kiwi was here.\"
  ")

  # Run the 🥝 script.
  sys.exec("kiwi ${path}")
catch (err)
  println "An error occurred: ${err}"
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
