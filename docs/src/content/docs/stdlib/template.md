---
title: template
description: Fill {{placeholder}} templates from a hashmap.
---

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

```kiwi nocheck
email = template::render_file("templates/welcome.txt", {name: "Ada", plan: "Pro"})
```

Notes:

- Placeholders are written without spaces: `{{name}}`, not `{{ name }}`.
- A placeholder with no matching key is left as it is.
- Values are inserted as they are, with no HTML escaping. Escape user-supplied text yourself before putting it into HTML.

For `printf`-style formatting, see [`fmt`](/kiwi/stdlib/fmt/); for most one-off strings, interpolation (`"Hello, ${name}"`) is simplest.

<!-- api: template -->
