# kiwi 🥝

Kiwi is a high-level, dynamically-typed language with an expressive syntax and versatile standard library, designed for efficiency across diverse tasks.

## Table of Contents

- [Getting Started](#getting-started)
  - [Docker](#docker)
  - [Linux](#linux-builds)
  - [Windows (WSL2)](#windows-wsl2)
  - [VS Code](#visual-studio-code-extension)
- [Documentation](#documentation)
  - [Wiki](#kiwi-wiki)
  - [Tests](#test-suite)
  - [Examples](#examples)
- [Contributions](#contributions)
- [License](#license)

## Getting Started

To start using Kiwi, please follow the instructions below. 

### Docker

Experiment with Kiwi in a Docker container.

```bash
sudo docker build -t kiwi-lang .
sudo docker run -it -v $(pwd):/workspace kiwi-lang
```

### Linux Builds

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

#### Installing Kiwi on Linux

Download the repository and run `sudo make install` to build Kiwi and run the installation script.

```bash
git clone https://github.com/fuseraft/kiwi.git
cd kiwi
sudo make install
```

##### To manually install Kiwi after it is built, run:

```bash
./bin/kiwi configure install
```

##### To uninstall Kiwi, run:

```bash
kiwi configure uninstall
```

### Windows (WSL2)

To run Kiwi on Windows, you can use [WSL2 (Windows Subsystem for Linux 2)](https://learn.microsoft.com/en-us/windows/wsl/). This provides a full Linux environment on Windows, allowing you to build and run Kiwi as if on a native Linux system.

#### Setting Up WSL2

1. **Install WSL2**: Open PowerShell as Administrator and run:
   ```powershell
   wsl --install
   ```
   This command installs WSL2 with the default Linux distribution (usually Ubuntu). Restart your computer if prompted.

2. **Install Development Tools**: Open WSL and install the necessary tools:
   ```bash
   sudo apt update
   sudo apt install g++ make
   ```

3. **Clone and Build Kiwi**: Clone the Kiwi repository and build it using `make`:
   ```bash
   git clone https://github.com/fuseraft/kiwi.git
   cd kiwi
   make
   ```

4. **Run Kiwi**: After building, you can run Kiwi directly in WSL:
   ```bash
   ./bin/kiwi
   ```

*Note: WSL2 provides a fully-featured Linux environment, so Kiwi runs natively in this environment without the need for Windows-specific adjustments.*

### Visual Studio Code Extension

You can install the [extension](https://marketplace.visualstudio.com/items?itemName=fuseraft.kiwi-lang) for syntax-highlighting in VS Code.

Launch VS Code Quick Open (<kbd>Ctrl</kbd>+<kbd>P</kbd>), paste the following command, and press enter.
```
ext install fuseraft.kiwi-lang
```

## Documentation

### Kiwi Wiki

You can find detailed information on language features in the [Kiwi Wiki](docs/README.md).

### Test Suite

Explore the [test suite](test.🥝) for a collection of tests.

To run the test suite, execute:

```shell
kiwi test
```

To build and run the test suite, execute:

```shell
make test
```

### Examples

You can find [many code examples](examples/) in the documentation.

Here are a handful:
- [Levenshtein distance](examples/algo/levenshtein.kiwi)
- [MD5 hashing algorithm](examples/algo/md5_hash.kiwi)
- [Sieve of Eratosthenes](examples/algo/sieve_of_eratosthenes.kiwi)
- [Conway's Game of Life](examples/cellular_automata/life.kiwi)
- [Project Euler stuff](examples/project_euler/).

## Contributions

I welcome and appreciate any and all contributions to the Kiwi project! Here's how you can contribute:

1. **Join the Discord**: [The Kiwi Programming Language](https://discord.com/channels/1221516965743431841/1221553678104920195)
2. **Fork the Repository**: Fork the project to your GitHub account.
3. **Clone the Forked Repository**: Clone it to your machine.
4. **Set Up Your Environment**: Follow the "Getting Started" section.
5. **Make Changes**: Implement your features or fixes.
6. **Test Your Changes**: Ensure all tests pass.
7. **Create a Pull Request**: Submit it from your fork to the main Kiwi repository.

For more details, please read [CONTRIBUTING.md](CONTRIBUTING.md).

## License

This project is licensed under the [MIT License](LICENSE).