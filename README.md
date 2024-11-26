# kiwi 🥝

Kiwi is a high-level, dynamically-typed language designed for efficiency and expressiveness across a variety of tasks.

## Table of Contents

- [Getting Started](#getting-started)
  - [Docker](#docker)
  - [Linux](#linux-builds)
  - [Windows (WSL2)](#windows-wsl2)
  - [VS Code Integration](#visual-studio-code-integration)
- [Documentation](#documentation)
  - [Wiki](#kiwi-wiki)
  - [Test Suite](#test-suite)
  - [Examples](#examples)
- [Contributing](#contributing)
- [License](#license)

## Getting Started

### Docker

Quickly experiment with Kiwi in a Docker container.

```bash
sudo docker build -t kiwi-lang .
sudo docker run -it -v $(pwd):/workspace kiwi-lang
```

### Linux Builds

Kiwi builds on Linux with GCC and Make. Use the commands below to install dependencies and build Kiwi.

#### Fedora / RHEL

```bash
sudo dnf update
sudo dnf install gcc-c++ make libffi-devel
```

#### Ubuntu / Debian

```bash
sudo apt update
sudo apt-get install g++ make libffi-dev
```

#### Build with Make

After installing the dependencies, clone the repository and build Kiwi:

```bash
git clone https://github.com/fuseraft/kiwi.git
cd kiwi
make
```

#### Installing Kiwi on Linux

To install Kiwi after building, run:

```bash
sudo make install
```

To uninstall Kiwi, use:

```bash
kiwi configure uninstall
```

### Windows (WSL2)

To run Kiwi on Windows, use [Windows Subsystem for Linux (WSL2)](https://learn.microsoft.com/en-us/windows/wsl/), which provides a full Linux environment on Windows.

#### Setting Up WSL2

1. **Install WSL2**: Open PowerShell as Administrator and run:
   ```powershell
   wsl --install
   ```
   This command installs WSL2 with a default Linux distribution (usually Ubuntu). Restart if prompted.

2. **Install Development Tools**: Open WSL and run:
   ```bash
   sudo apt update
   sudo apt install g++ make libffi-dev
   ```

3. **Clone and Build Kiwi**: Inside WSL, clone the Kiwi repository and build it with `make`:
   ```bash
   git clone https://github.com/fuseraft/kiwi.git
   cd kiwi
   make
   ```

4. **Run Kiwi**: After building, you can run Kiwi directly in WSL:
   ```bash
   ./bin/kiwi
   ```

*Note: WSL2 enables Kiwi to run natively without Windows-specific modifications.*

### Visual Studio Code Integration

For syntax highlighting and code snippets in Visual Studio Code, install the [Kiwi language extension](https://marketplace.visualstudio.com/items?itemName=fuseraft.kiwi-lang).

Open VS Code, launch Quick Open (<kbd>Ctrl</kbd>+<kbd>P</kbd>), paste the following command, and press Enter:

```
ext install fuseraft.kiwi-lang
```

## Documentation

### Kiwi Wiki

The [Kiwi Wiki](docs/README.md) provides comprehensive information on language features, usage, and standard library functions.

### Test Suite

Explore Kiwi’s [test suite](test.🥝) for a variety of test cases. To run the test suite:

```shell
kiwi test
```

To build and run the test suite using Make:

```bash
make test
```

### Examples

Find [examples](examples/) demonstrating Kiwi’s capabilities, including:

- [Levenshtein distance](examples/algo/levenshtein.kiwi)
- [MD5 hashing algorithm](examples/algo/md5_hash.kiwi)
- [Sieve of Eratosthenes](examples/algo/sieve_of_eratosthenes.kiwi)
- [Conway's Game of Life](examples/cellular_automata/life.kiwi)
- [Project Euler examples](examples/project_euler/)

## Contributing

Contributions are highly appreciated! Here’s how to get involved:

1. **Join the Discussion**: Join the Kiwi community on [Discord](https://discord.gg/9PW3857Bxs).
2. **Fork the Repository**: Fork Kiwi on GitHub.
3. **Clone the Repository**: Clone your forked copy to your machine.
4. **Set Up Your Environment**: Follow the steps in "Getting Started."
5. **Make Changes**: Implement new features or fix issues.
6. **Test Your Changes**: Run all tests to ensure stability.
7. **Submit a Pull Request**: Submit your changes for review.

For more details, please refer to [CONTRIBUTING.md](CONTRIBUTING.md).

## License

This project is licensed under the [MIT License](LICENSE).