# Interprocess Communication

This is an example of implementing interprocess communication with a text file.

The example demonstrates a client application that writes to a file `./queue.ipc` and a server application that listens for changes to `./queue.ipc`, taking some action on it.

To start the server, run:

```bash
kiwi server
```

To start a client, run:

```bash
kiwi client
```