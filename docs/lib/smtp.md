# `smtp`

The `smtp` package provides functions for sending email via SMTP, including a fluent `Message` builder.

---

## Package Functions

### `send(host, port, from_addr, recipients, subject, body, username?, password?, use_tls?)`

Sends an email in a single call.

**Parameters**

| Type              | Name         | Description                                      | Default  |
|-------------------|--------------|--------------------------------------------------|----------|
| `string`          | `host`       | SMTP server hostname                             | —        |
| `integer`         | `port`       | SMTP server port                                 | —        |
| `string`          | `from_addr`  | Sender email address                             | —        |
| `string\|list`    | `recipients` | Recipient address or list of addresses           | —        |
| `string`          | `subject`    | Email subject line                               | —        |
| `string`          | `body`       | Email body text                                  | —        |
| `string`          | `username`   | SMTP authentication username                     | `""`     |
| `string`          | `password`   | SMTP authentication password                     | `""`     |
| `boolean`         | `use_tls`    | Use TLS/STARTTLS                                 | `true`   |

**Returns**

| Type      | Description            |
|-----------|------------------------|
| `boolean` | `true` on success      |

---

### `message(from_addr, subject, body)`

Creates a fluent `Message` builder.

**Parameters**

| Type     | Name        | Description            |
|----------|-------------|------------------------|
| `string` | `from_addr` | Sender email address   |
| `string` | `subject`   | Email subject          |
| `string` | `body`      | Email body             |

**Returns** `Message`

---

## `Message`

A fluent email builder returned by `smtp::message()`.

### `Message.new(from_addr, subject, body)`

Direct constructor (prefer `smtp::message()` factory function).

---

### `.add_to(addr)`

Adds a recipient (`To:`).

**Parameters**

| Type     | Name   | Description       |
|----------|--------|-------------------|
| `string` | `addr` | Email address     |

**Returns** `Message` (self)

---

### `.add_cc(addr)`

Adds a CC recipient.

**Parameters**

| Type     | Name   | Description       |
|----------|--------|-------------------|
| `string` | `addr` | Email address     |

**Returns** `Message` (self)

---

### `.send(host, port?, username?, password?, use_tls?)`

Sends the message.

**Parameters**

| Type      | Name       | Description                      | Default  |
|-----------|------------|----------------------------------|----------|
| `string`  | `host`     | SMTP server hostname              | —        |
| `integer` | `port`     | SMTP server port                  | `587`    |
| `string`  | `username` | SMTP username                     | `""`     |
| `string`  | `password` | SMTP password                     | `""`     |
| `boolean` | `use_tls`  | Use TLS/STARTTLS                  | `true`   |

**Returns**

| Type      | Description       |
|-----------|-------------------|
| `boolean` | `true` on success |

---

## Examples

### Quick send

```kiwi
import "smtp"

smtp::send(
  "smtp.example.com", 587,
  "sender@example.com",
  "recipient@example.com",
  "Hello from Kiwi",
  "This is the message body.",
  "sender@example.com", "p@ssw0rd"
)
```

### Fluent builder

```kiwi
import "smtp"

smtp::message("alerts@example.com", "Build Failed", "The CI pipeline failed on main.")
  .add_to("dev@example.com")
  .add_to("ops@example.com")
  .add_cc("manager@example.com")
  .send("smtp.example.com", 587, "alerts@example.com", "p@ssw0rd")
```

### Multiple recipients via list

```kiwi
import "smtp"

recipients = ["alice@example.com", "bob@example.com", "carol@example.com"]

smtp::send(
  "smtp.example.com", 587,
  "newsletter@example.com",
  recipients,
  "Weekly Update",
  "Here is this week's update...",
  "newsletter@example.com", "secret"
)
```

### Using port 465 (SSL)

```kiwi
import "smtp"

smtp::send(
  "smtp.gmail.com", 465,
  "me@gmail.com", "you@example.com",
  "Test", "Sent from Kiwi.",
  "me@gmail.com", "app-password"
)
```
