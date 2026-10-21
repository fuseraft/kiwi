---
title: smtp
description: Send email through an SMTP server.
---

The `smtp` package sends email through an SMTP server, either in one call or with a fluent `Message` builder.

```kiwi nocheck
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

```kiwi nocheck
smtp::message("alerts@example.com", "Build failed", "The CI pipeline failed on main.")
  .add_to("dev@example.com")
  .add_to("ops@example.com")
  .add_cc("manager@example.com")
  .send("smtp.example.com", 587, "alerts@example.com", "p@ssw0rd")
```

TLS is on by default and is negotiated with STARTTLS, as on port 587; pass `use_tls: false` for servers without TLS. Servers that only accept implicit TLS (typically port 465) are not supported.

Keep credentials out of source code: read them with [`env::get`](/kiwi/stdlib/env/) or load them with [`dotenv`](/kiwi/stdlib/dotenv/). `smtp` is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode).

<!-- api: smtp -->
