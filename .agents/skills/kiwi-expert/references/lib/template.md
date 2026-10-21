# template

> Fill {{placeholder}} templates from a hashmap.

The `template` package replaces `{{name}}` placeholders in a string with values from a hashmap.

```kiwi
message = template::render(
  "Hello, {{name}}! You have {{count}} new messages.",
  {name: "Ada", count: 3}
)
println message   # Hello, Ada! You have 3 new messages.
```

`render_each` renders one template per row and returns the results as a list:

```kiwi
rows = [{name: "Ada", email: "ada@example.com"}, {name: "Grace", email: "grace@example.com"}]
println template::render_each("{{name}} <{{email}}>", rows)
# ["Ada <ada@example.com>", "Grace <grace@example.com>"]
```

`render_file(path, values)` reads the template from a file:

```kiwi
email = template::render_file("templates/welcome.txt", {name: "Ada", plan: "Pro"})
```

Notes:

- Placeholders are written without spaces: `{{name}}`, not `{{ name }}`.
- A placeholder with no matching key is left as it is.
- Values are inserted as they are, with no HTML escaping. Escape user-supplied text yourself before putting it into HTML.

For `printf`-style formatting, see [`fmt`](fmt.md); for most one-off strings, interpolation (`"Hello, ${name}"`) is simplest.

## Functions

### `template::render`

```kiwi
template::render(tmpl, vars)
```

Renders a template string, replacing {{key}} with values from a hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `tmpl` | `string` |  | Template string containing {{key}} placeholders. |
| `vars` | `hashmap` |  | Map of variable names to replacement values. |

**Returns** `string` — The rendered string.

```kiwi
import template
result = template::render("Hello, {{name}}! You are {{age}} years old.", {
  "name": "Alice",
  "age": 30
})
println result  # Hello, Alice! You are 30 years old.
```

### `template::render_file`

```kiwi
template::render_file(path, vars)
```

Renders a template file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | Path to the template file. |
| `vars` | `hashmap` |  | Map of variable names to replacement values. |

**Returns** `string` — The rendered string.

### `template::render_each`

```kiwi
template::render_each(tmpl, rows)
```

Renders a template string with a list of hashmaps (repeated section).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `tmpl` | `string` |  | Template string with {{key}} placeholders. |
| `rows` | `list` |  | A list of hashmaps, one per output row. |

**Returns** `list` — A list of rendered strings, one per row.

```kiwi
import template
rows = template::render_each("{{id}}: {{name}}", [
  { "id": 1, "name": "Alice" },
  { "id": 2, "name": "Bob" }
])
println rows.join("\n")
```
