# `template`

The `template` package provides a lightweight string template engine. Templates use `{{key}}` placeholders that are replaced with values from a hashmap.

---

## Package Functions

### `render(tmpl, vars)`

Renders a template string, replacing every `{{key}}` occurrence with the corresponding value from `vars`.

**Parameters**

| Type      | Name   | Description                                   |
|-----------|--------|-----------------------------------------------|
| `string`  | `tmpl` | Template string with `{{key}}` placeholders   |
| `hashmap` | `vars` | Map of variable names to replacement values   |

**Returns**

| Type     | Description          |
|----------|----------------------|
| `string` | The rendered string  |

Values are converted to strings via `.to_string()` automatically.

---

### `render_file(path, vars)`

Reads a template from a file and renders it with the given variables.

**Parameters**

| Type      | Name   | Description                       |
|-----------|--------|-----------------------------------|
| `string`  | `path` | Path to the template file         |
| `hashmap` | `vars` | Map of variable names to values   |

**Returns**

| Type     | Description         |
|----------|---------------------|
| `string` | The rendered string |

---

### `render_each(tmpl, rows)`

Renders a template once per row in a list, returning a list of rendered strings.

**Parameters**

| Type     | Name   | Description                                      |
|----------|--------|--------------------------------------------------|
| `string` | `tmpl` | Template string with `{{key}}` placeholders      |
| `list`   | `rows` | List of hashmaps — one hashmap per output line   |

**Returns**

| Type   | Description                             |
|--------|-----------------------------------------|
| `list` | List of rendered strings, one per row   |

---

## Examples

### Basic substitution

```kiwi
import "template"

result = template::render("Hello, {{name}}! You are {{age}} years old.", {
  "name": "Alice",
  "age": 30
})

println result   # Hello, Alice! You are 30 years old.
```

### HTML template

```kiwi
import "template"

html = template::render("<h1>{{title}}</h1><p>{{body}}</p>", {
  "title": "Welcome",
  "body":  "Thanks for visiting."
})

println html
```

### Render from file

Given `email.txt`:
```
Subject: {{subject}}

Hello {{name}},

{{message}}

Regards,
{{sender}}
```

```kiwi
import "template"

text = template::render_file("email.txt", {
  "subject": "Your order shipped",
  "name":    "Bob",
  "message": "Your order #4521 is on its way.",
  "sender":  "Support Team"
})

println text
```

### Batch rendering

```kiwi
import "template"

rows = template::render_each("{{id}}: {{name}} <{{email}}>", [
  { "id": 1, "name": "Alice", "email": "alice@example.com" },
  { "id": 2, "name": "Bob",   "email": "bob@example.com"   },
  { "id": 3, "name": "Carol", "email": "carol@example.com" }
])

println rows.join("\n")
# 1: Alice <alice@example.com>
# 2: Bob <bob@example.com>
# 3: Carol <carol@example.com>
```

### Combining with fio for report generation

```kiwi
import "template"
import "fio"

data = fio::read_text("report_template.html")
output = template::render(data, {
  "title":   "Monthly Summary",
  "month":   "March",
  "revenue": "42,000"
})

fio::write_text("report_march.html", output)
```
