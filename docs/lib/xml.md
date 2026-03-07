# `xml`

The `xml` package provides an XML parser and serializer for Kiwi programs. It parses XML strings or files into a tree of `XmlNode` structs that can be queried, navigated, and serialized back to XML.

```kiwi
import "xml"
```

---

## Package Functions

### `xml::parse(input)`

Parses an XML-formatted string and returns the document root element as an `XmlNode`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | A valid XML string. |

**Returns**

| Type | Description |
| :--- | :--- |
| `XmlNode` | The root element of the parsed document. |

**Example**

```kiwi
import "xml"

src = "<book><title>Kiwi</title><author>Scotty</author></book>"
root = xml::parse(src)
println root.tag  # prints: book
```

---

### `xml::parse_file(path)`

Reads an XML file from disk and parses it, returning the root `XmlNode`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `path` | Path to the XML file. |

**Returns**

| Type | Description |
| :--- | :--- |
| `XmlNode` | The root element of the parsed document. |

**Throws**

A string error if the file does not exist.

**Example**

```kiwi
import "xml"

root = xml::parse_file("/data/config.xml")
println root.tag
```

---

## `XmlNode` Struct

Every parsed XML element, text node, comment, CDATA section, and processing instruction is represented as an `XmlNode`.

### Fields

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `tag` | The element tag name, or one of the special values `"#text"`, `"#comment"`, `"#cdata"`, `"#pi"`. |
| `hashmap` | `attrs` | Attribute map (name -> value). Empty for non-element nodes. |
| `list` | `children` | Child `XmlNode` instances. Empty for leaf nodes. |
| `string` | `content` | Text payload for `#text`, `#comment`, `#cdata`, and `#pi` nodes. Empty for element nodes. |

### Special Node Types

| Tag | Represents |
| :--- | :--- |
| `"#text"` | A text node — `content` holds the text. |
| `"#comment"` | An XML comment (`<!-- ... -->`). |
| `"#cdata"` | A CDATA section (`<![CDATA[...]]>`). |
| `"#pi"` | A processing instruction (`<?name ...?>`). The `attrs` hashmap holds `"name"`. |

---

## `XmlNode` Methods

### `get_attr(name, default?)`

Returns the value of an attribute by name. Returns the default if the attribute is not present.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `string` | `name` | The attribute name. | — |
| `string` | `default` | Fallback value when the attribute is absent. | `""` |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The attribute value, or the default. |

**Example**

```kiwi
import "xml"

src = "<img src=\"photo.png\" alt=\"A photo\"/>"
root = xml::parse(src)
println root.get_attr("src")          # prints: photo.png
println root.get_attr("alt")          # prints: A photo
println root.get_attr("width", "0")   # prints: 0
```

---

### `find(tag_name)`

Returns the first direct child element whose tag matches `tag_name`, or `null` if none is found.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `tag_name` | The tag name to search for. |

**Returns**

| Type | Description |
| :--- | :--- |
| `XmlNode`\|`null` | The first matching child, or `null`. |

**Example**

```kiwi
import "xml"

src = "<root><name>Kiwi</name><version>1.0</version></root>"
root = xml::parse(src)
node = root.find("name")
println node.text()  # prints: Kiwi
```

---

### `find_all(tag_name)`

Returns a list of all direct child elements whose tag matches `tag_name`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `tag_name` | The tag name to search for. |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | All direct children with the given tag. May be empty. |

**Example**

```kiwi
import "xml"

src = "<items><item>a</item><item>b</item><note>misc</note></items>"
root = xml::parse(src)
items = root.find_all("item")
println items.size()   # prints: 2
for item in items do
  println item.text()
end
# prints: a, b
```

---

### `find_all_deep(tag_name)`

Recursively searches all descendants (depth-first) and returns every node whose tag matches `tag_name`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `tag_name` | The tag name to search for. |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | All matching nodes at any depth, in depth-first order. |

**Example**

```kiwi
import "xml"

src = "<a><b><c>deep</c></b><c>shallow</c></a>"
root = xml::parse(src)
all_c = root.find_all_deep("c")
println all_c.size()        # prints: 2
println all_c[0].text()     # prints: deep
println all_c[1].text()     # prints: shallow
```

---

### `elements()`

Returns a list of direct child nodes that are elements — that is, all children except `#text`, `#comment`, `#cdata`, and `#pi` nodes.

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | Element children only. |

**Example**

```kiwi
import "xml"

src = "<root>Some text<child/></root>"
root = xml::parse(src)
println root.children.size()   # prints: 2  (text node + child element)
println root.elements().size() # prints: 1  (child element only)
```

---

### `text()`

Returns the concatenated text content of all direct `#text` children. Does not recurse into nested elements.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The joined text of direct text children. |

**Example**

```kiwi
import "xml"

src = "<p>Hello, <b>world</b>!</p>"
root = xml::parse(src)
println root.text()          # prints: Hello, !   (only direct text nodes)
println root.find("b").text() # prints: world
```

---

### `text_content()`

Returns the concatenated text of all descendant `#text` nodes, recursively. Use this when you want the full visible text under a node regardless of nesting.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | All descendant text joined together. |

**Example**

```kiwi
import "xml"

src = "<p>Hello, <b>world</b>!</p>"
root = xml::parse(src)
println root.text_content()  # prints: Hello, world!
```

---

### `serialize(indent?)`

Serializes the node and its descendants back to an XML string. Pass an indentation level to produce pretty-printed output.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `integer` | `indent` | The current indentation level (2 spaces per level). | `0` |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The XML string representation of this node. |

**Example**

```kiwi
import "xml"

src = "<root><child>value</child></root>"
root = xml::parse(src)
println root.serialize()
# prints:
# <root>
#   <child>value</child>
# </root>
```

---

### `to_string()`

Alias for `serialize(0)`. Returns the node serialized as a compact XML string.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The XML string representation of this node. |

**Example**

```kiwi
import "xml"

src = "<note><text>Hi</text></note>"
root = xml::parse(src)
println root.to_string()
# prints: <note>\n  <text>Hi</text>\n</note>
```

---

## Complete Example

```kiwi
import "xml"

src = "
<library>
  <book id=\"1\" genre=\"fiction\">
    <title>The Great Kiwi</title>
    <author>Scotty</author>
  </book>
  <book id=\"2\" genre=\"technical\">
    <title>Kiwi in Practice</title>
    <author>Fuseraft</author>
  </book>
</library>
"

root = xml::parse(src)
println root.tag  # prints: library

# Iterate over all book elements
for book in root.find_all("book") do
  id    = book.get_attr("id")
  genre = book.get_attr("genre")
  title = book.find("title").text()
  auth  = book.find("author").text()
  println id + ": " + title + " by " + auth + " (" + genre + ")"
end
# prints:
# 1: The Great Kiwi by Scotty (fiction)
# 2: Kiwi in Practice by Fuseraft (technical)

# Serialize back to XML
println root.serialize()
```
