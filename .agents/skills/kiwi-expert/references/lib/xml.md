# xml

> Parse, query, build and serialize XML.

The `xml` package parses XML into a tree of `XmlNode` values, which you can search, modify and serialize back to text.

## Parsing and querying

```kiwi
catalog = xml::parse('
<catalog>
  <item id="1"><name>Kiwi</name><price>0.5</price></item>
  <item id="2"><name>Mango</name><price>1.25</price></item>
</catalog>')

items = catalog.find_all("item")
println items.size()                     # 2
println items[0].attr("id")              # 1
println items[0].find("name").text()     # Kiwi
println catalog.find_all_deep("price").map(do (p) => p.text())   # ["0.5", "1.25"]
```

`xml::parse_file(path)` parses a file.

| Method | Returns |
| :-- | :-- |
| `find(tag)` | The first child element with that tag, or `null`. |
| `find_all(tag)` | The child elements with that tag. |
| `find_all_deep(tag)` | Every descendant element with that tag. |
| `elements()` | The child elements, skipping text and comments. |
| `first_child()`, `last_child()` | The first or last child node. |
| `text()` | The text directly inside the element. |
| `text_content()` | All text inside the element, including descendants. |
| `attr(name, default)` | An attribute value. |
| `has_attr`, `set_attr`, `remove_attr` | Inspect or change attributes. |
| `is_element()`, `is_text()` | The node kind. |

Every node has `tag`, `attrs`, `children` and `content` fields. Text, comments, CDATA sections and processing instructions are nodes with the tags `#text`, `#comment`, `#cdata` and `#pi`, and their text in `content`.

## Building XML

```kiwi
config = xml::create("config", {version: "2"})

host = xml::create("host")
host.add_child(xml::text_node("localhost"))
config.add_child(host)

println xml::stringify(config)
# <config version="2">
#   <host>localhost</host>
# </config>
```

Add text with `text_node`, comments with `comment_node`, and CDATA with `cdata_node`. `stringify(node, indent)` and `node.serialize(indent)` produce indented XML; `indent` is the starting indentation level.

## Pipelines

`xml::pipeline()` turns repeated elements into a list of hashmaps, one per element, with attributes and child element text as fields. It works like the [`csv`](csv.md#pipelines-with-typed-columns) and [`json`](json.md#pipelines) pipelines:

```kiwi
rows = xml::pipeline()
  .select("item")
  .with_schema({"id": "integer", "price": "float"})
  .from_string('<c><item id="1"><name>Kiwi</name><price>0.5</price></item></c>')
  .parse()

println rows   # [{"id": 1, "name": "Kiwi", "price": 0.5}]
```

`select(tag)` chooses the element to extract for the sources added after it. As with the other pipelines, `as_dataset(name)` names a source, and `parse()` then returns a hashmap of datasets.

## Functions

### `xml::parse`

```kiwi
xml::parse(input: string): any
```

Parses an XML string and returns the root element.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string` |  | The XML-formatted string to parse. |

**Returns** `XmlNode` — The document root element.

**Throws** `string` — On malformed XML.

### `xml::parse_file`

```kiwi
xml::parse_file(file_path: string): any
```

Parses an XML file and returns the root element.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `file_path` | `string` |  | The path to the XML file. |

**Returns** `XmlNode` — The document root element.

**Throws** `string` — If the file does not exist or XML is malformed.

### `xml::create`

```kiwi
xml::create(tag: string, attrs: hashmap = {}, content: string = ""): any
```

Creates a new element node.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `tag` | `string` |  | The element tag name. |
| `attrs` | `hashmap` | `{}` | Attribute map (default `{}`). |
| `content` | `string` | `""` | Text content (default `""`), added as a `#text` child. |

**Returns** `any` — XmlNode

### `xml::text_node`

```kiwi
xml::text_node(content: string): any
```

Creates a `#text` node.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `content` | `string` |  | The text content. |

**Returns** `any` — XmlNode

### `xml::comment_node`

```kiwi
xml::comment_node(content: string): any
```

Creates a `#comment` node.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `content` | `string` |  | The comment text. |

**Returns** `any` — XmlNode

### `xml::cdata_node`

```kiwi
xml::cdata_node(content: string): any
```

Creates a `#cdata` node.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `content` | `string` |  | The CDATA content. |

**Returns** `any` — XmlNode

### `xml::stringify`

```kiwi
xml::stringify(node: any, indent: integer = 0): string
```

Serializes an XmlNode to an XML string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `node` | `any` |  | The node to serialize. |
| `indent` | `integer` | `0` | Indentation level (default `0`). |

**Returns** `string` — string

### `xml::pipeline`

```kiwi
xml::pipeline(): XmlPipeline
```

Creates a new XmlPipeline builder for fluent XML record extraction.

**Returns** `XmlPipeline` — A new pipeline builder instance.

```kiwi
# Extract all <item> elements with type coercion
rows = xml::pipeline()
  .select("item")
  .with_schema({"id": "integer", "price": "float", "active": "boolean"})
  .from_file("catalog.xml")
  .parse()

# Multiple named datasets
data = xml::pipeline()
  .select("user").from_file("users.xml").as_dataset("users")
  .select("role").from_file("roles.xml").as_dataset("roles")
  .parse()

users = data["users"]
roles = data["roles"]
```

## `XmlNode` struct

Represents a parsed XML node (element, text, comment, CDATA, or PI).

### `XmlNode.new`

```kiwi
XmlNode.new(tag, attrs, children, content)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `tag` | `any` |  |  |
| `attrs` | `any` |  |  |
| `children` | `any` |  |  |
| `content` | `any` |  |  |

### `.get_attr`

```kiwi
get_attr(name, default = "")
```

Returns the value of an attribute, or a default if not present.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The attribute name. |
| `default` | `any` | `""` | The fallback value (default `""`). |

**Returns** `string` — The attribute value or default.

### `.attr`

```kiwi
attr(name, default = "")
```

Alias for `get_attr`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `any` |  |  |
| `default` | `any` | `""` |  |

### `.has_attr`

```kiwi
has_attr(name)
```

Returns true if the attribute exists.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The attribute name. |

**Returns** boolean

### `.set_attr`

```kiwi
set_attr(name, value)
```

Sets or updates an attribute.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The attribute name. |
| `value` | `string` |  | The attribute value. |

### `.remove_attr`

```kiwi
remove_attr(name)
```

Removes an attribute.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The attribute name. |

### `.add_child`

```kiwi
add_child(node)
```

Appends a child node.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `node` | `XmlNode` |  | The node to append. |

### `.is_element`

```kiwi
is_element()
```

Returns true if this is an element node (not #text, #comment, #cdata, or #pi).

**Returns** boolean

### `.is_text`

```kiwi
is_text()
```

Returns true if this is a `#text` node.

**Returns** boolean

### `.find`

```kiwi
find(tag_name)
```

Returns the first direct child element with the given tag name.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `tag_name` | `string` |  | The tag to search for. |

**Returns** `XmlNode|null` — The first matching child, or null.

### `.find_all`

```kiwi
find_all(tag_name)
```

Returns all direct child elements with the given tag name.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `tag_name` | `string` |  | The tag to search for. |

**Returns** `list` — Matching child nodes.

### `.find_all_deep`

```kiwi
find_all_deep(tag_name)
```

Recursively searches all descendants for nodes with the given tag name.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `tag_name` | `string` |  | The tag to search for. |

**Returns** `list` — All matching nodes in depth-first order.

### `.elements`

```kiwi
elements()
```

Returns only element children (excludes `#text`, `#comment`, `#cdata`, `#pi`).

**Returns** `list` — Element child nodes.

### `.first_child`

```kiwi
first_child()
```

Returns the first element child, or null.

**Returns** XmlNode|null

### `.last_child`

```kiwi
last_child()
```

Returns the last element child, or null.

**Returns** XmlNode|null

### `.text`

```kiwi
text()
```

Returns the concatenated text of all direct `#text` children.

**Returns** `string` — Joined text content.

### `.text_content`

```kiwi
text_content()
```

Returns the concatenated text of all descendant `#text` nodes (deep).

**Returns** `string` — Full recursive text content.

### `.serialize`

```kiwi
serialize(indent = 0)
```

Serializes the node back to an XML string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `indent` | `integer` | `0` | The current indentation level (default `0`). |

**Returns** `string` — The XML string representation.

### `.to_string`

```kiwi
to_string()
```

## `XmlPipeline` struct

A fluent builder for extracting structured records from one or more XML sources.

### `XmlPipeline.new`

```kiwi
XmlPipeline.new()
```

Initialises an empty pipeline builder.

### `.select`

```kiwi
select(tag: string): XmlPipeline
```

Sets the element tag name to extract as records from the next source.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `tag` | `string` |  | The XML element tag name to select. |

**Returns** `XmlPipeline` — self, for chaining.

### `.with_schema`

```kiwi
with_schema(schema: hashmap): XmlPipeline
```

Sets a type-coercion schema for the next source.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `schema` | `hashmap` |  | Map of field name to type string. |

**Returns** `XmlPipeline` — self, for chaining.

### `.from_file`

```kiwi
from_file(path: string): XmlPipeline
```

Registers an XML file as a source using the current pending settings.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | Path to the XML file. |

**Returns** `XmlPipeline` — self, for chaining.

**Throws** `string` — If the file does not exist at parse time.

### `.from_string`

```kiwi
from_string(input: string): XmlPipeline
```

Registers an in-memory XML string as a source using the current pending settings.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string` |  | A valid XML string. |

**Returns** `XmlPipeline` — self, for chaining.

### `.as_dataset`

```kiwi
as_dataset(name: string): XmlPipeline
```

Names the most recently registered source as a dataset.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The dataset key name. |

**Returns** `XmlPipeline` — self, for chaining.

**Throws** `string` — If called before any source is registered.

### `.parse`

```kiwi
parse()
```

Executes the pipeline and returns the parsed results.

**Returns** `list|hashmap` — Parsed XML data.

**Throws** `string` — On missing file or mixed dataset/non-dataset sources.
