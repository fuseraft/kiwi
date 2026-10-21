---
title: dotenv
description: Load .env files into the process environment.
---

The `dotenv` package reads `KEY=VALUE` files, conventionally named `.env`, and loads them into the environment so the rest of the program can read them with [`env::get`](/kiwi/stdlib/env/).

```kiwi nocheck
dotenv::load()                 # reads ./.env
port = env::get("PORT")

dotenv::load("config/.env.production", false)   # don't overwrite existing variables
```

`load` returns the values it read. `parse_file` and `parse` return the values without touching the environment:

```kiwi
settings = dotenv::parse("PORT=8080\nNAME=\"kiwi app\"  # display name\n# a comment")
println settings   # {"PORT": "8080", "NAME": "kiwi app"}
```

Values are always strings.

## File format

| Form | Example | Notes |
| :-- | :-- | :-- |
| Plain | `KEY=value` | Surrounding whitespace is trimmed. |
| Double quoted | `KEY="hello world"` | Keeps whitespace; supports `\"`, `\\`, `\n`, `\t` and `\r`. |
| Single quoted | `KEY='literal $value'` | Taken literally. |
| Inline comment | `KEY=value  # note` | Everything from ` #` on is ignored. |
| `export` prefix | `export KEY=value` | The `export` is ignored. |
| Comment | `# comment` | Ignored, as are blank lines. |

<!-- api: dotenv -->
