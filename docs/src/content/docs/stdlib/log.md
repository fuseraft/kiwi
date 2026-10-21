---
title: log
description: Leveled logging to stderr and, optionally, a file.
---

The `log` package writes leveled log messages to stderr, and optionally to a file as well. Messages below the current level are dropped.

```kiwi stderr
log::info("service starting")
log::debug("hidden: the default level is INFO")

log::set_level("DEBUG")
log::debug("now visible")
log::warn("disk usage above 80%")
log::error("could not reach database")
```

```text
[INFO] [3/22/2026 6:00:00 PM] service starting
[DEBUG] [3/22/2026 6:00:00 PM] now visible
[WARN] [3/22/2026 6:00:00 PM] disk usage above 80%
[ERROR] [3/22/2026 6:00:00 PM] could not reach database
```

## Levels

From least to most severe: `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`. `set_level` shows messages at that level and above; the default is `INFO`.

`log::fatal(message)` logs the message and then ends the program with exit code `1`.

## Options

```kiwi nocheck
log::set_file("/var/log/myapp.log")   # also append every message to this file
log::timestamps(false)                # drop the timestamp: "[INFO] message"
```

<!-- api: log -->
