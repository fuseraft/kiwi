# kiwi 🥝

Kiwi is a high-level, dynamically-typed language designed for efficiency and expressiveness across a variety of tasks.

## Table of Contents

- [Getting Started](#getting-started)
  - [VS Code Integration](#visual-studio-code-integration)
- [Documentation](#documentation)
  - [Kiwi Docs](#kiwi-docs)
  - [Test Suite](#test-suite)
  - [Examples](#examples)
- [Contributing](#contributing)
- [License](#license)

## Getting Started

You should visit the [docs](docs/README.md) to learn the fundamentals of the language.

Clone the repository:
```bash
$ git clone https://github.com/fuseraft/kiwi.git
```

Build Kiwi:
```bash
$ ./build.sh
```

Run the test suite:
```bash
$ ./bin/kiwi tests/test
```

### Visual Studio Code Integration

For syntax highlighting and code snippets in VS Code, install the [Kiwi language extension](https://marketplace.visualstudio.com/items?itemName=fuseraft.kiwi-lang).

Open VS Code, launch Quick Open (<kbd>Ctrl</kbd>+<kbd>P</kbd>), paste the following command, and press Enter:

```
ext install fuseraft.kiwi-lang
```

## Documentation

### Kiwi Docs

The [docs](docs/README.md) provide comprehensive information on language features, usage, and the [standard library](docs/lib/README.md).

### Test Suite

Explore the [test suite](tests/test.kiwi) for a variety of test cases. To run the test suite:

```shell
$ kiwi tests/test
```

### Examples

Find [examples](scripts/) demonstrating Kiwi, including:

- [Levenshtein distance](scripts/algo/levenshtein.kiwi)
- [The cryptography library](lib/crypto.kiwi)
- [Sieve of Eratosthenes](scripts/algo/sieve.kiwi)
- [Conway's Game of Life](scripts/cellular_automata/life.kiwi)
- [Project Euler examples](scripts/project_euler/)

## Contributing

Contributions are highly appreciated! Here’s how to get involved:

1. **Join the Discussion**: Join the community on [Discord](https://discord.gg/9PW3857Bxs).
2. **Fork the Repository**: Fork Kiwi on GitHub.
3. **Clone the Repository**: Clone your forked copy to your machine.
4. **Set Up Your Environment**: Follow the steps in "Getting Started."
5. **Make Changes**: Implement new features or fix issues.
6. **Test Your Changes**: Run all tests to ensure stability.
7. **Submit a Pull Request**: Submit your changes for review.

For more details, please refer to [CONTRIBUTING.md](CONTRIBUTING.md).

## License

This project is licensed under the [MIT License](LICENSE).