---
title: Installation
description: Install Kiwi with the installer script, build it from source, or run it in Docker.
---

Kiwi runs on Linux, macOS and Windows. The installers build it from source, so you need the [.NET 10 SDK](https://dotnet.microsoft.com/download/dotnet/10.0) first.

## Install script

**Linux and macOS:**

```sh
curl -sSL https://raw.githubusercontent.com/fuseraft/kiwi/main/install.sh | bash
```

**Windows (PowerShell):**

```powershell
irm https://raw.githubusercontent.com/fuseraft/kiwi/main/install.ps1 | iex
```

The installer builds Kiwi, copies the binary and the standard library into place, and adds `kiwi` to your `PATH`. Open a new terminal and check that it worked:

```sh
kiwi --version
```

### Installer options

Download `install.sh` (or `install.ps1`) and pass options to change where and how Kiwi is installed:

| Option | Effect |
| :-- | :-- |
| `--user` | Install for the current user in `~/.kiwi` (the default). |
| `--system` | Install system-wide in `/opt/kiwi` and link `/usr/local/bin/kiwi`. Needs `sudo`. |
| `--prefix=PATH` | Install into a directory of your choice. |
| `--url=URL` | Download a pre-built binary instead of building from source. |
| `--use-existing` | Install the binary already in `./bin/` instead of building. |
| `--update` | Remove the current install and install the latest version. |
| `--uninstall` | Remove Kiwi. |

```sh
./install.sh --system
./install.sh --prefix=/usr/local
./install.sh --uninstall
```

## Build from source

```sh
git clone https://github.com/fuseraft/kiwi.git
cd kiwi
./build.sh
```

`build.sh` detects your platform, tries an ahead-of-time (AOT) native build, and falls back to a trimmed self-contained build if AOT is not available. Either way the result is `./bin/kiwi`. To target a specific platform, pass a runtime identifier: `linux-x64`, `linux-arm64`, `osx-x64`, `osx-arm64` or `win-x64`.

```sh
./build.sh linux-arm64
```

On Windows, use `build.ps1` instead.

To run `kiwi` from anywhere without the installer, link the binary onto your `PATH`:

```sh
mkdir -p ~/bin
ln -sf "$(pwd)/bin/kiwi" ~/bin/kiwi
```

:::note
The binary finds the standard library through `kiwi-settings.json`, which sits next to it and points at `../lib`. Keep `bin/` and `lib/` side by side, or use [`--stdlib-path`](/kiwi/tools/cli/#options) to point somewhere else.
:::

### Run the test suite

The test suite is written in Kiwi. Run it after building to make sure everything works:

```sh
./bin/kiwi tests/test
```

It exits with `0` when every test passes.

## Docker

The repository includes a `Dockerfile` that builds Kiwi and packages it with the standard library:

```sh
docker build -t kiwi .
docker run --rm -v "$(pwd)":/scripts kiwi /scripts/hello.kiwi
docker run --rm -it kiwi    # start the REPL
```

## Editor support

The [Kiwi extension for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=fuseraft.kiwi-lang) adds syntax highlighting and snippets. Install it from the Quick Open prompt (<kbd>Ctrl</kbd>+<kbd>P</kbd>):

```text
ext install fuseraft.kiwi-lang
```

## Packages

[Zest](https://github.com/fuseraft/zest) is Kiwi's package manager. It installs packages from GitHub with semver constraints and a lock file:

```sh
zest install owner/repo
```

## Next steps

Take [the tour](/kiwi/getting-started/tour/) for a quick look at the whole language.
