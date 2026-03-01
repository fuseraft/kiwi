# Building Kiwi

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