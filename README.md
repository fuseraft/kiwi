# Kiwi 🥝

Kiwi is a modern, lightweight scripting language built for expressiveness and utility.

## Installation

**Linux / macOS** — one-liner installer:
```bash
curl -sSL https://raw.githubusercontent.com/fuseraft/kiwi/main/install.sh | bash
```

**Windows** — run in PowerShell:
```powershell
irm https://raw.githubusercontent.com/fuseraft/kiwi/main/install.ps1 | iex
```

Both installers download the latest release for your platform, verify its checksum, and add `kiwi` to your PATH. The release binaries are self-contained, so you don't need .NET. Open a new terminal and you're ready:
```bash
kiwi --help
```

### Installer options

| Flag | Description |
|------|-------------|
| `--user` | Install for current user only — `~/.kiwi` (default) |
| `--system` | Install system-wide — `/opt/kiwi`, symlinked to `/usr/local/bin` |
| `--prefix=PATH` | Install to a custom directory |
| `--version=X.Y.Z` | Install a specific release instead of the latest |
| `--source` | Build from source (requires the [.NET 10 SDK](https://dotnet.microsoft.com/download/dotnet/10.0)) |
| `--url=URL` | Download a binary from a URL, with the standard library from `main` |
| `--update` | Update to the latest version |
| `--uninstall` | Remove Kiwi from the system |

```bash
# Examples
./install.sh --system          # system-wide install (Linux/macOS)
./install.sh --version=1.8.0   # a specific release
./install.sh --prefix=/usr/local
./install.sh --uninstall
```

You can also download an archive for your platform from the [releases page](https://github.com/fuseraft/kiwi/releases) and unpack it anywhere. Linux binaries need glibc 2.34+ (Ubuntu 22.04, Debian 12, RHEL 9 or newer) and `libicu`.

### Manual install (build from source)

```bash
git clone https://github.com/fuseraft/kiwi.git
cd kiwi
./build.sh
```

### Docker

```bash
docker build -t kiwi .
docker run --rm -v $(pwd):/scripts kiwi /scripts/hello.kiwi
```

## Visual Studio Code Integration

For syntax highlighting and code snippets in VS Code, install the [Kiwi language extension](https://marketplace.visualstudio.com/items?itemName=fuseraft.kiwi-lang).

Open VS Code, launch Quick Open (<kbd>Ctrl</kbd>+<kbd>P</kbd>), paste the following command, and press Enter:

```
ext install fuseraft.kiwi-lang
```

## Package Manager

[Zest](https://github.com/fuseraft/zest) is the official package manager for Kiwi.
Install community packages directly from GitHub with semver constraints and a lock file.

```bash
zest install owner/repo
```

## Documentation

The [documentation site](https://fuseraft.ai/kiwi/) covers the language, the tools, and the [standard library](https://fuseraft.ai/kiwi/stdlib/). Its source lives in [`docs/`](docs/); see [`docs/README.md`](docs/README.md) to build it locally.

There are also some [examples](/examples/) you can run!

## Test Suite

Explore the [test suite](tests/lib/suite) for a variety of test cases. To run the test suite:

```shell
$ kiwi tests/test
```

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