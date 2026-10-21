---
title: xml
description: Parse, query, build and serialize XML.
---

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

`xml::pipeline()` turns repeated elements into a list of hashmaps, one per element, with attributes and child element text as fields. It works like the [`csv`](/kiwi/stdlib/csv/#pipelines-with-typed-columns) and [`json`](/kiwi/stdlib/json/#pipelines) pipelines:

```kiwi
rows = xml::pipeline()
  .select("item")
  .with_schema({"id": "integer", "price": "float"})
  .from_string('<c><item id="1"><name>Kiwi</name><price>0.5</price></item></c>')
  .parse()

println rows   # [{"id": 1, "name": "Kiwi", "price": 0.5}]
```

`select(tag)` chooses the element to extract for the sources added after it. As with the other pipelines, `as_dataset(name)` names a source, and `parse()` then returns a hashmap of datasets.

<!-- api: xml -->
