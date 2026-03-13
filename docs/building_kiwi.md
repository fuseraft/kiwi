# Building Kiwi

## From Source

Clone the repository:
```bash
$ git clone https://github.com/fuseraft/kiwi.git
```

Run the build script:
```bash
$ ./build.sh
```

Create a symlink to run `kiwi` from anywhere:
```bash
$ mkdir -p ~/bin
$ ln -sf "$(pwd)/bin/kiwi" ~/bin/kiwi
```

Verify setup:
```bash
$ kiwi -v
```

## Using Docker

Build the image:
```bash
$ docker build -t kiwi .
```

Run a script:
```bash
$ docker run --rm -v $(pwd):/scripts kiwi /scripts/hello.kiwi
```

Drop into the REPL:
```bash
$ docker run --rm -it kiwi
```