---
title: env
description: Environment variables and command-line arguments.
---

The `env` package reads and writes environment variables and gives access to the arguments a script was started with.

## Environment variables

```kiwi
env::set("APP_MODE", "test")
println env::get("APP_MODE")         # test
println env::get("NOT_SET").empty()  # true: missing variables read as ""
println typeof(env::all())           # hashmap
```

`env::set` is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode). To load variables from a `.env` file, see [`dotenv`](/kiwi/stdlib/dotenv/).

## Command-line arguments

| Function | Returns |
| :-- | :-- |
| `env::args()` | The raw arguments, in order. |
| `env::argv()` | The arguments as a hashmap: `-key=value` becomes `key: "value"`, `--flag` becomes `flag: "true"`, and other arguments map to themselves. |
| `env::opt(key)` | The value of a `-key=value` argument. |
| `env::parse_args(args, value_options)` | A hashmap of flags, where options listed in `value_options` take the following argument as their value. |

```kiwi
config = env::parse_args(["--mode", "fast", "-v"], ["--mode"])
println config   # {"--mode": "fast", "-v": true}
```

See [Passing arguments to scripts](/kiwi/tools/cli/#passing-arguments-to-scripts) for examples, and [`cli::CliParser`](/kiwi/stdlib/cli/#parsing-arguments) for a fuller argument parser.

<!-- api: env -->
