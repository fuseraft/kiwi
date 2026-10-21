# smtp

> Send email through an SMTP server.

The `smtp` package sends email through an SMTP server, either in one call or with a fluent `Message` builder.

```kiwi
smtp::send(
  "smtp.example.com", 587,
  "sender@example.com",             # from
  "recipient@example.com",          # to: an address or a list of addresses
  "Hello from Kiwi",                # subject
  "This is the message body.",      # body
  "sender@example.com", "p@ssw0rd"  # username and password
)
```

The builder suits messages with several recipients:

```kiwi
smtp::message("alerts@example.com", "Build failed", "The CI pipeline failed on main.")
  .add_to("dev@example.com")
  .add_to("ops@example.com")
  .add_cc("manager@example.com")
  .send("smtp.example.com", 587, "alerts@example.com", "p@ssw0rd")
```

TLS is on by default and is negotiated with STARTTLS, as on port 587; pass `use_tls: false` for servers without TLS. Servers that only accept implicit TLS (typically port 465) are not supported.

Keep credentials out of source code: read them with [`env::get`](env.md) or load them with [`dotenv`](dotenv.md). `smtp` is unavailable in [safe mode](../tools/cli.md#safe-mode).

## Functions

### `smtp::send`

```kiwi
smtp::send(host, port, from_addr, recipients, subject, body, username = "", password = "", use_tls = true)
```

Sends an email in one call.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `host` | `string` |  | SMTP server hostname. |
| `port` | `integer` |  | SMTP server port. |
| `from_addr` | `string` |  | Sender email address. |
| `recipients` | `string\|list` |  | Recipient address or list of addresses. |
| `subject` | `string` |  | Email subject. |
| `body` | `string` |  | Email body. |
| `username` | `string` | `""` | SMTP username (default ""). |
| `password` | `string` | `""` | SMTP password (default ""). |
| `use_tls` | `boolean` | `true` | Whether to use TLS (default true). |

**Returns** `boolean` — true on success.

```kiwi
import smtp
smtp::send("smtp.example.com", 587, "me@example.com", "you@example.com",
  "Hello", "This is the body.", "me@example.com", "s3cr3t")
```

### `smtp::message`

```kiwi
smtp::message(from_addr, subject, body)
```

Creates a fluent Message builder.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `from_addr` | `string` |  | Sender email address. |
| `subject` | `string` |  | Email subject. |
| `body` | `string` |  | Email body. |

**Returns** Message

## `Message` struct

A fluent email message builder.

### `Message.new`

```kiwi
Message.new(from_addr, subject, body)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `from_addr` | `any` |  |  |
| `subject` | `any` |  |  |
| `body` | `any` |  |  |

### `.add_to`

```kiwi
add_to(addr)
```

Adds a recipient.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `addr` | `string` |  | Email address. |

**Returns** `Message` — self

### `.add_cc`

```kiwi
add_cc(addr)
```

Adds a CC recipient.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `addr` | `string` |  | Email address. |

**Returns** `Message` — self

### `.send`

```kiwi
send(host, port = 587, username = "", password = "", use_tls = true)
```

Sends the message via SMTP.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `host` | `string` |  | SMTP server hostname. |
| `port` | `integer` | `587` | SMTP server port (default 587). |
| `username` | `string` | `""` | SMTP username (default ""). |
| `password` | `string` | `""` | SMTP password (default ""). |
| `use_tls` | `boolean` | `true` | Whether to use TLS/STARTTLS (default true). |

**Returns** `boolean` — true on success.
