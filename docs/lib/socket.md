# `socket`

The `socket` package provides a set of functions and constants for network communication using sockets in Kiwi. It allows you to create servers and clients that can send and receive data over TCP/IP networks.

## Table of Contents

- [Package Overview](#package-overview)
- [Constants](#constants)
  - [Address Families](#address-families)
  - [Socket Types](#socket-types)
  - [Shutdown Modes](#shutdown-modes)
- [Function Details](#function-details)
  - [`create`](#create)
  - [`bind`](#bind)
  - [`listen`](#listen)
  - [`accept`](#accept)
  - [`connect`](#connect)
  - [`send`](#send)
  - [`receive`](#receive)
  - [`close`](#close)
  - [`shutdown`](#shutdown)
- [Example Usage](#example-usage)
  - [Server Example](#server-example)
  - [Client Example](#client-example)
- [Important Notes](#important-notes)
- [Additional Examples](#additional-examples)
  - [Echo Server](#echo-server)

## **Package Overview**

The `socket` package includes:

- **Constants** for address families, socket types, and shutdown modes.
- **Functions** for creating sockets, binding, listening, accepting connections, connecting to servers, sending and receiving data, and closing sockets.

---

## **Constants**

### Address Families

- **`AF_INET`**: Address family for IPv4 addresses. Use this for most standard internet connections.
  ```kiwi
  const socket::AF_INET = 2
  ```

### Socket Types

- **`SOCK_STREAM`**: Provides sequenced, reliable, two-way, connection-based byte streams. This is used for TCP connections.
  ```kiwi
  const socket::SOCK_STREAM = 1
  ```
- **`SOCK_DGRAM`**: Supports datagrams (connectionless, unreliable messages of a fixed maximum length). This is used for UDP connections.
  ```kiwi
  const socket::SOCK_DGRAM = 2
  ```

### Shutdown Modes

- **`SHUT_RD`**: Disables further receive operations.
  ```kiwi
  const socket::SHUT_RD = 0
  ```
- **`SHUT_WR`**: Disables further send operations.
  ```kiwi
  const socket::SHUT_WR = 1
  ```
- **`SHUT_RDWR`**: Disables further send and receive operations.
  ```kiwi
  const socket::SHUT_RDWR = 2
  ```

---

## **Function Details**

### `create`

Creates a new socket and returns its unique socket ID.

#### **Syntax:**
```kiwi
socket::create(family: integer = socket::AF_INET, 
               type: integer = socket::SOCK_STREAM, 
               protocol: integer = 0): integer
```

#### **Parameters:**

- **`family`** *(optional)*: The address family.
  - **Default**: `socket::AF_INET`
  - **Example**: `socket::AF_INET`
- **`type`** *(optional)*: The socket type.
  - **Default**: `socket::SOCK_STREAM`
  - **Example**: `socket::SOCK_STREAM` for TCP, `socket::SOCK_DGRAM` for UDP
- **`protocol`** *(optional)*: The protocol number.
  - **Default**: `0` (selects the default protocol for the given family and type)

#### **Returns:**

- **`integer`**: A unique socket ID representing the created socket.

#### **Example:**
```kiwi
# Create a TCP socket using IPv4
sock_id = socket::create()
```

---

### `bind`

Binds a socket to an address and port.

#### **Syntax:**
```kiwi
socket::bind(sock_id: integer, address: string, port: integer)
```

#### **Parameters:**

- **`sock_id`**: The socket ID returned by `socket::create`.
- **`address`**: The IP address to bind to.
  - **Example**: `"127.0.0.1"` for localhost
- **`port`**: The port number to bind the socket to.
  - **Range**: Typically between `0` and `65535`

#### **Example:**
```kiwi
# Bind the socket to localhost on port 8080
socket::bind(sock_id, "127.0.0.1", 8080)
```

---

### `listen`

Listens for incoming connections on a socket.

#### **Syntax:**
```kiwi
socket::listen(sock_id: integer, backlog: integer = 5)
```

#### **Parameters:**

- **`sock_id`**: The socket ID.
- **`backlog`** *(optional)*: The maximum number of queued connections.
  - **Default**: `5`
  - **Note**: This defines the length of the queue for pending connections.

#### **Example:**
```kiwi
# Start listening for incoming connections
socket::listen(sock_id, 5)
```

---

### `accept`

Accepts an incoming connection on a listening socket.

#### **Syntax:**
```kiwi
socket::accept(sock_id: integer): hashmap
```

#### **Parameters:**

- **`sock_id`**: The socket ID that is listening for connections.

#### **Returns:**

- **`hashmap`**: A hashmap containing:
  - **`"client_sock_id"`**: The socket ID for the accepted client connection.
  - **`"client_address"`**: The client's IP address as a string.
  - **`"client_port"`**: The client's port number as an integer.

#### **Example:**
```kiwi
# Accept a client connection
client_info = socket::accept(sock_id)
client_sock = client_info["client_sock_id"]
```

---

### `connect`

Connects a socket to a remote address and port.

#### **Syntax:**
```kiwi
socket::connect(sock_id: integer, address: string, port: integer)
```

#### **Parameters:**

- **`sock_id`**: The socket ID.
- **`address`**: The remote IP address to connect to.
- **`port`**: The remote port number.

#### **Example:**
```kiwi
# Connect to a server on localhost port 8080
socket::connect(sock_id, "127.0.0.1", 8080)
```

---

### `send`

Sends data over a connected socket.

#### **Syntax:**
```kiwi
socket::send(sock_id: integer, data: any): integer
```

#### **Parameters:**

- **`sock_id`**: The socket ID.
- **`data`**: The data to send. Can be a string or a list of bytes.

#### **Returns:**

- **`integer`**: The number of bytes sent.

#### **Example:**
```kiwi
# Send a message to the connected socket
bytes_sent = socket::send(sock_id, "Hello, World!")
```

---

### `receive`

Receives data from a connected socket.

#### **Syntax:**
```kiwi
socket::receive(sock_id: integer, length: integer): string
```

#### **Parameters:**

- **`sock_id`**: The socket ID.
- **`length`**: The maximum amount of data to receive in bytes.

#### **Returns:**

- **`string`**: The data received from the socket.

#### **Example:**
```kiwi
# Receive up to 1024 bytes of data
data = socket::receive(sock_id, 1024)
```

---

### `close`

Closes a socket.

#### **Syntax:**
```kiwi
socket::close(sock_id: integer)
```

#### **Parameters:**

- **`sock_id`**: The socket ID to close.

#### **Example:**
```kiwi
# Close the socket
socket::close(sock_id)
```

---

### `shutdown`

Shuts down part or all of a full-duplex connection on a socket.

#### **Syntax:**
```kiwi
socket::shutdown(sock_id: integer, how: integer = socket::SHUT_RDWR)
```

#### **Parameters:**

- **`sock_id`**: The socket ID.
- **`how`** *(optional)*: Specifies how the socket should be shut down.
  - **Default**: `socket::SHUT_RDWR`
  - **Options**:
    - `socket::SHUT_RD` (Disables further receive operations)
    - `socket::SHUT_WR` (Disables further send operations)
    - `socket::SHUT_RDWR` (Disables both send and receive operations)

#### **Example:**
```kiwi
# Shut down both send and receive operations
socket::shutdown(sock_id)
```

---

## **Example Usage**

### Server Example

[Click here](../../examples/sockets/server.kiwi) to view the source code.

```kiwi
fn main()
  # Create a server socket
  server_sock = socket::create(socket::AF_INET, socket::SOCK_STREAM)
  
  # Bind the socket to localhost on port 8080
  socket::bind(server_sock, "127.0.0.1", 8080)
  
  # Start listening for incoming connections
  socket::listen(server_sock, 5)
  println("Server is listening on 127.0.0.1:8080")
  
  # Accept a client connection
  client_info = socket::accept(server_sock)
  client_sock = client_info["client_sock_id"]
  client_address = client_info["client_address"]
  client_port = client_info["client_port"]
  
  println("Accepted connection from ${client_address}:${client_port}")
  
  # Receive data from the client
  data = socket::receive(client_sock, 1024)
  println("Received data: ${data}")
  
  # Send data to the client
  socket::send(client_sock, "Hello, client!")
  
  # Close the client socket
  socket::close(client_sock)
  
  # Close the server socket
  socket::close(server_sock)
end

main()
```

---

### Client Example

[Click here](../../examples/sockets/client.kiwi) to view the source code.

```kiwi
fn main()
  # Create a client socket
  client_sock = socket::create(socket::AF_INET, socket::SOCK_STREAM)
  
  # Connect to the server on localhost port 8080
  socket::connect(client_sock, "127.0.0.1", 8080)
  println("Connected to server on 127.0.0.1:8080")
  
  # Send data to the server
  socket::send(client_sock, "Hello, server!")
  
  # Receive data from the server
  data = socket::receive(client_sock, 1024)
  println("Received data: ${data}")
  
  # Close the client socket
  socket::close(client_sock)
end

main()
```

---

## **Important Notes**

1. **Blocking Operations**: By default, socket operations like `accept`, `receive`, and `connect` may block the program's execution until they complete.

2. **Error Handling**: Always check the return values of socket functions where applicable to handle errors gracefully.

3. **Data Encoding**: When sending and receiving data, ensure that both the client and server agree on the data format and encoding (e.g., strings, binary data, etc).

4. **Resource Management**: Always close sockets using `socket::close` when they are no longer needed to free up system resources.

5. **Port Numbers**: Ports below 1024 are considered privileged and may require additional permissions to bind to.

---

## **Additional Examples**

### Echo Server

```kiwi
fn main()
  server_sock = socket::create()
  socket::bind(server_sock, "127.0.0.1", 8080)
  socket::listen(server_sock)

  println("Echo server listening on port 8080")

  while true do
    client_info = socket::accept(server_sock)
    client_sock = client_info["client_sock_id"]
    client_address = client_info["client_address"]
    client_port = client_info["client_port"]

    println("Connection from ${client_address}:${client_port}")

    data = socket::receive(client_sock, 1024)
    println("Received: ${data}")

    socket::send(client_sock, data)
    socket::close(client_sock)
  end
end

main()
```