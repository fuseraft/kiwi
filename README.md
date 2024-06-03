# kiwi 🥝

An unorthodox scripting language.

## Table of Contents

- [Getting Started](#getting-started)
  - [Docker](#docker)
  - [Linux Builds](#linux-builds)
    - [Fedora / RHEL](#fedora--rhel)
    - [Ubuntu / Debian](#ubuntu--debian)
    - [Installing Kiwi on Linux](#installing-kiwi-on-linux)
      - [To manually install Kiwi after it is built, run:](#to-manually-install-kiwi-after-it-is-built-run)
      - [To uninstall Kiwi, run:](#to-uninstall-kiwi-run)
  - [Windows Builds](#windows-builds)
  - [Visual Studio Code Extension](#visual-studio-code-extension)
- [Documentation](#documentation)
  - [Kiwi Wiki](#kiwi-wiki)
  - [Test Suite](#test-suite)
  - [Code Examples](#code-examples)
    - [Algorithms](#algorithms)
    - [Cellular Automata](#cellular-automata)
    - [Project Euler](#project-euler)
    - [Web Application](#web-application)
    - [Temporary Script](#temporary-script)
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

### Windows Builds

You can find a 64-bit build (named **`kiwi.exe`**) in the latest release.

To build on Windows, you need to open a [64-bit hosted developer command prompt](https://learn.microsoft.com/en-us/cpp/build/how-to-enable-a-64-bit-visual-cpp-toolset-on-the-command-line?view=msvc-170).

Then navigate to the source code and run [`build.bat`](build.bat).

```cmd
cd path\to\kiwi
build.bat
```

*Note: The Windows build does not support `.🥝` files (use the `.kiwi` extension).*

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

Explore the [tests](tests/) directory for a collection of test scripts. 

To run the test suite, execute:

```shell
kiwi test
```

To build and run the test suite, execute:

```shell
make test
```

### Code Examples

#### Algorithms

- [Levenshtein Distance](examples/algo/levenshtein.kiwi)
- [MD5](examples/algo/md5_hash.kiwi)
- [Sieve of Eratosthenes](examples/algo/sieve_of_eratosthenes.kiwi)

#### Cellular Automata

- [Rule 30](examples/cellular_automata/rule30.kiwi)
- [Conway's Game of Life](examples/cellular_automata/life.kiwi)
- [Brian's Brain](examples/cellular_automata/brain.kiwi)

#### Project Euler

- [Project Euler examples](examples/project_euler/).

#### Web Application

Below is a simple HTTP web application. You can find the [example project here](examples/webapp/app.kiwi).

```kiwi
import "@kiwi/web" as web
import "@kiwi/fs" as fs

# HTML helpers
html = {
  "shared": fs.read("templates/shared.html"),
  "index": fs.read("templates/index.html"),
  "contact": fs.read("templates/contact.html")
}

# A simple template engine.
fn build_html(data)
  return html.shared.replace("<%content%>", data)
end

# GET /
web.get(["/", "/index"], with (req) do
  return web.ok(build_html(html.index), "text/html")
end)

# GET /contact
web.get("/contact", with (req) do
  return web.ok(build_html(html.contact), "text/html")
end)

# POST /contact
web.post("/contact", with (req) do  
  println("Received content from client:\nbody: ${req.body}\nparams: ${req.params}")
  return web.redirect("/")
end)

# serve static content
web.public("/", "./public")

# server and port configuration
host = "0.0.0.0", port = 8080

# start the web server
println("Starting Kiwi Web Server at http://${host}:${port}")
web.listen(host, port)
```

#### Temporary Script

Below is a script that generates a temporary script and executes it.

```kiwi
import "@kiwi/fs" as fs
import "@kiwi/math" as math
import "@kiwi/sys" as sys

try
  # Look for a temporary directory. Fail fast.
  if !fs.exists(fs.tmpdir())
    println("Could not find temporary directory.")
    exit(1)
  end
  
  # Generate a random temporary file path.
  filename = math.random("0123456789ABCDEF", 10)
  path = fs.combine(fs.tmpdir(), "${filename}.kiwi")

  # Write a little 🥝 script.
  fs.write(path, "
    /#
    This script will delete itself and let you know it was there.
    #/
    import \"@kiwi/fs\" as fs
    fs.remove(\"${path}\")
    println(\"Kiwi was here running as ${fs.filename(path)}.\")
  ")

  # Run the 🥝 script.
  sys.exec("kiwi ${path}")
catch (err)
  println("An error occurred: ${err}")
end
```

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
