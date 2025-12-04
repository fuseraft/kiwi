# Project Kiwi 🥝

Kiwi is a high-level, dynamically-typed language designed for efficiency and expressiveness across a variety of tasks.

## Getting Started

You should visit the [docs](docs/README.md) to learn the fundamentals of the language.

Clone the repository:
```bash
git clone https://github.com/fuseraft/kiwi.git
```

Build Kiwi:
```bash
./build.sh
```

Run the test suite:
```bash
./bin/kiwi scripts/test
```

## VS Code Extension

For syntax highlighting and code snippets in Visual Studio Code, install the [VS Code extension](https://marketplace.visualstudio.com/items?itemName=fuseraft.kiwi-lang).

## Sample Code

Below is a simple markdown generator that reads `.kiwi` files and generates a file called `kiwi-files.md` with their contents. 

You can find other examples in the [docs](docs/README.md) and in the [scripts](scripts/) directory. 

```kiwi
/#
This simple markdown generator reads all `.kiwi` files found within the current 
directory and its subdirectories, and generates a markdown file called `output.md` 
containing the contents of each file.
#/

fn generate_markdown(output_name: string = "output.md",
                     ext:         string = ".cs",
                     lang:        string = "csharp")
  try
    # get the absolute path to the output file (in the current directory)
    var (output_path: string = fio::abspath("./${output_name}"))
    
    # remove the file if it exists
    fio::remove(output_path)

    # loop through each file, and build some markdown with it.
    var (markdown: list = [])
    for path in fio::glob("./", ["./**/*${ext}"]) do
      var (filename:  string = fio::filename(path),
           content:   string = fio::read(path))

      markdown.push(["## ${filename}", "```${lang}", content, "```"])
    end

    # write the markdown content to the output file
    fio::writeln(output_path, markdown.flatten())

    # throw an error if we could not generate the file
    throw "could not generate ${output_path}" 
      when !fio::exists(output_path)
    
    println "generated ${output_path}"
  catch (err)
    println "an error occurred: ${err}"
  end
end

generate_markdown("kiwi-files.md", ".kiwi", "kiwi")
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