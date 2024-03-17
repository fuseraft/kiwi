# Kiwi 🥝

Kiwi is a dynamically-typed, general-purpose scripting language.

## Table of Contents

1. [Getting Started](#getting-started)
    1. [Docker](#docker)
    2. [Linux](#linux-builds)
        1. [Fedora / RHEL](#fedora--rhel)
        2. [Ubuntu / Debian](#ubuntu--debian)
        3. [Installing Kiwi on Linux](#installing-kiwi-on-linux)
    3. [Windows](#windows-builds)
    4. [Visual Studio Code Extension](#visual-studio-code-extension)
2. [Documentation](#documentation)
    1. [Test Suite](#test-suite)
    1. [Example Code](#example-code)
        1. [Project Euler Examples](#project-euler-examples)
        2. [Example Script](#example-script)
        3. [Example Web Application](#example-web-application)
3. [Contributions](#contributions)
4. [License](#license)

## Getting Started

To start using Kiwi, please follow the instructions below. 

### Docker

Experiment with Kiwi in a Docker container.

```bash
sudo docker build -t kiwi-lang .
sudo docker run -it -v $(pwd):/workspace kiwi-language
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

Clone and build Kiwi.  Run `sudo make install` to build Kiwi and run the installation script.

```bash
git clone https://github.com/fuseraft/kiwi.git
cd kiwi
sudo make install
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

For detailed information on language features, refer to the [docs](docs/README.md).

### Test Suite

Explore the [tests](tests/) directory for a collection of test scripts. 

To run the test suite, execute:

```shell
make test
```

### Example Code

#### Project Euler Examples

For fun, I wrote some [Project Euler examples](examples/project_euler/).

#### Example Script
Below is an example script that generates a temporary script and executes it.

```ruby
import "@kiwi/fs" as fs
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
    import \"@kiwi/fs\" as fs
    fs.remove(\"${@path}\")
    println \"Kiwi was here.\"
  ")

  # Run the 🥝 script.
  sys.exec("kiwi ${path}")
catch (err)
  println "An error occurred: ${err}"
end
```

#### Example Web Application

Below is a simple HTTP web application. You can find the application [here](examples/webapp/web.🥝).

```ruby
import "@kiwi/web" as web
import "@kiwi/fs" as fs

##
# HTML helpers
##

def build_html(data) return "<html><body>${data}</body></html>" end
def build_tr(key, value) return "<tr><td>${key}</td><td>${value}</td></tr>" end

##
# GET endpoint controller actions
##

get_index = lambda(req) do
  content = "<div>The Kiwi Web Server is running.</div>
             <div>${fs.read("form.html")}</div></body>"
  return web.ok(build_html(content), "text/html")
end

##
# POST endpoint controller actions
##

post_index = lambda(req) do
  body = req["__BODY"], content = body
  
  println "Received content from client: ${body}"
  
  if req["Content-Type"] == "application/json"
    body = req["__BODY"].to_h()
    first_name = body.has_key("firstName") ? body["firstName"] : ""
    last_name = body.has_key("lastName") ? body["lastName"] : ""
    content = build_tr(first_name, last_name)
  end

  return web.ok(content, "text/plain")
end

##
# Route registration.
##

web.get(["/", "/index"], get_index)
web.post("/post", post_index)

# Configure host and port.
host = "0.0.0.0"
port = 8080

# Start web server.
println "Starting Kiwi Web Server at http://${host}:${port}"
web.listen(host, port)
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
