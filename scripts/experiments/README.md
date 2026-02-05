# ziputil.kiwi

A simple zip utility written in [The Kiwi Programming Language](https://github.com/fuseraft/kiwi).

## Usage

```sh
chmod +x ./ziputil.kiwi
./ziputil.kiwi <option(s)>
```

## Options

- `-c path`: Create a ZIP file from a specified path.
- `-x path`: Extract a ZIP file from a specified path.
- `-o path`: Specify the output path for the ZIP file.
- `-s`: Perform the operation quietly (suppress output).

## Example

### Create a ZIP file

```sh
./ziputil.kiwi -c /path/to/directory_or_file
```

### Extract a ZIP file

```sh
./ziputil.kiwi -x /path/to/archive.zip
```

### Create a ZIP file with a specified output path

```sh
./ziputil.kiwi -c /path/to/directory_or_file -o /path/to/output.zip
```

### Perform the operation quietly

```sh
./ziputil.kiwi -c /path/to/directory_or_file -s
```

## Help

To display the help screen, run the script without any options or with an invalid option.

```sh
./ziputil.kiwi
```

or

```sh
./ziputil.kiwi -h
```