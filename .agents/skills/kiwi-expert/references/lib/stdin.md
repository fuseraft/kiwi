# `stdin`

The `stdin` package provides functions for reading from standard input in Kiwi. These functions allow both blocking and non-blocking reads, as well as convenient ways to consume all available input.

---

## Package Functions

### `read()`
Reads all available input from stdin as a string until EOF or no more data is immediately available.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The input data as a string. |

**Example**

```kiwi
import "stdin"

# Usage: echo "hello world" | kiwi script.kiwi
var input = stdin::read()
println "You sent: " + input
```

---

### `readbytes()`
Reads all available input from stdin as raw bytes until EOF or no more data is immediately available.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `bytes` | The input data as bytes. |

**Example**

```kiwi
import "stdin"

# Usage: cat image.png | kiwi script.kiwi
var raw = stdin::readbytes()
println "Read " + raw.size().to_string() + " bytes"
```

---

### `readline()`
Reads a single line from stdin. The returned string does not include the trailing newline.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | A single line of input. |

**Example**

```kiwi
import "stdin"

# Prompt the user for their name interactively
print "Enter your name: "
var name = stdin::readline()
println "Hello, " + name + "!"
```

---

### `lines()`
Reads all complete lines currently available from stdin and returns them as a list of strings. Each string has its trailing newline removed.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of strings, each representing one available line. |

**Example**

```kiwi
import "stdin"

# Usage: printf "one\ntwo\nthree\n" | kiwi script.kiwi
var all_lines = stdin::lines()

println "Line count: " + all_lines.size().to_string()

for line, i in all_lines
  println i.to_string() + ": " + line
end
# 0: one
# 1: two
# 2: three
```

---

### `empty()`
Checks whether there is any data currently available on stdin.

**Parameters**
_None_

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if no data is available, `false` otherwise. |

**Example**

```kiwi
import "stdin"

# Usage: echo "some data" | kiwi script.kiwi
#   or: kiwi script.kiwi   (no piped input)
if stdin::empty()
  println "No input provided."
else
  var input = stdin::read()
  println "Received: " + input
end
```
