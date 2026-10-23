---
title: Installation
description: Install Kiwi with the installer script, download a release, build it from source, or run it in Docker.
---

Kiwi runs on Linux, macOS and Windows, on x64 and arm64. Each release has a prebuilt, self-contained binary for every platform, so you don't need .NET to run it.

## Install script

**Linux and macOS:**

```sh
curl -sSL https://raw.githubusercontent.com/fuseraft/kiwi/main/install.sh | bash
```

**Windows (PowerShell):**

```powershell
irm https://raw.githubusercontent.com/fuseraft/kiwi/main/install.ps1 | iex
```

The installer downloads the latest release for your platform, checks it against the release's SHA-256 checksums, installs the binary and the standard library, and adds `kiwi` to your `PATH`. Open a new terminal and check that it worked:

```sh
kiwi --version
```

### Installer options

Pass options to change where and how Kiwi is installed. With the one-line install, put them after `bash -s --`:

```sh
curl -sSL https://raw.githubusercontent.com/fuseraft/kiwi/main/install.sh | bash -s -- --version=1.8.0
```

| Option | Effect |
| :-- | :-- |
| `--user` | Install for the current user in `~/.kiwi` (the default). |
| `--system` | Install system-wide in `/opt/kiwi` and link `/usr/local/bin/kiwi`. Needs `sudo`. |
| `--prefix=PATH` | Install into a directory of your choice. |
| `--version=X.Y.Z` | Install that release instead of the latest. |
| `--source` | Build from source instead of downloading a release. Needs the [.NET 10 SDK](https://dotnet.microsoft.com/download/dotnet/10.0). |
| `--url=URL` | Download a binary from a URL, with the standard library from the `main` branch. |
| `--use-existing` | Install the binary already in `./bin/` instead of building. |
| `--update` | Remove the current install and install the latest version. |
| `--uninstall` | Remove Kiwi. |

`install.ps1` takes the same options in PowerShell form: `-System`, `-Prefix PATH`, `-Version X.Y.Z`, `-Source`, `-Update`, `-Uninstall`.

```sh
./install.sh --system
./install.sh --prefix=/usr/local
./install.sh --uninstall
```

If the latest release has no binary for your platform, the installer builds Kiwi from source when the .NET 10 SDK is installed. For an older release without one, pass `--source` with `--version`. A download that doesn't match its checksum, or a binary that can't start on your system, is never installed, and an existing install is left as it was.

## Download a release

Each [release](https://github.com/fuseraft/kiwi/releases) has an archive for each platform, and a `SHA256SUMS` file to check them against:

| Platform | Archive |
| :-- | :-- |
| Linux x64 | `kiwi-linux-x64.tar.gz` |
| Linux arm64 | `kiwi-linux-arm64.tar.gz` |
| macOS Apple silicon | `kiwi-osx-arm64.tar.gz` |
| macOS Intel | `kiwi-osx-x64.tar.gz` |
| Windows x64 | `kiwi-win-x64.zip` |

Each archive holds a `kiwi` directory with `bin/` and `lib/`. Unpack it anywhere and run `kiwi/bin/kiwi`:

```sh
curl -sSLO https://github.com/fuseraft/kiwi/releases/latest/download/kiwi-linux-x64.tar.gz
tar -xzf kiwi-linux-x64.tar.gz
./kiwi/bin/kiwi --version
```

The Linux binaries need glibc 2.34 or later, as on Ubuntu 22.04, Debian 12 and RHEL 9 or newer, and the ICU library (`libicu`), which most distributions install by default. Alpine and other musl-based distributions aren't supported.

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
