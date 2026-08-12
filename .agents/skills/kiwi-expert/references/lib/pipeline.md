# `pipeline`

The `pipeline` package provides a DSL for building named, step-based data pipelines. Each pipeline passes a shared context hashmap through a sequence of named steps. Optional hooks let you observe or time each step, and an error handler lets you recover from failures without crashing the pipeline.

---

## Structs

### `Pipeline`

Returned by `pipeline::build()`. Holds the step list and optional hooks.

| Method | Description |
| :--- | :--- |
| `step(name, action)` | Appends a named step. `action` is a lambda accepting the context hashmap. |
| `before_each(hook)` | Registers a hook called before every step: `hook(name, ctx)`. |
| `after_each(hook)` | Registers a hook called after every successful step: `hook(name, ctx)`. |
| `on_error(handler)` | Registers an error recovery handler: `handler(name, err, ctx)`. If absent, errors re-throw and halt the pipeline. |
| `run(ctx)` | Executes all steps in order and returns the final context. `ctx` defaults to `{}`. |

---

## Package Functions

### `build(name)`

Creates and returns a new empty `Pipeline`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `name` | A label for the pipeline, used in logs and error messages. |

**Returns**

| Type | Description |
| :--- | :--- |
| `Pipeline` | A new empty pipeline. |

---

## Examples

### Basic ETL pipeline

```kiwi
p = pipeline::build("etl")

p.step("load", do (ctx)
  ctx["rows"] = [
    {name: "Alice", age: 32, active: true},
    {name: "Bob",   age: 17, active: true},
    {name: "Carol", age: 25, active: false}
  ]
end)

p.step("filter", do (ctx)
  rows = ctx["rows"]
  ctx["rows"] = rows.filter(do (r) => r["active"] && r["age"] >= 18 end)
end)

p.step("names", do (ctx)
  rows = ctx["rows"]
  ctx["names"] = rows.map(do (r) => r["name"] end)
end)

result = p.run({})
names = result["names"]
println names  # ["Alice"]
```

### Before/after hooks

```kiwi
p = pipeline::build("timed")

p.before_each(do (name, ctx)
  ctx["__t"] = time::ticks()
end)

p.after_each(do (name, ctx)
  t0 = ctx["__t"]
  elapsed = time::ticksms(time::ticks() - t0)
  println("${name}: ${elapsed}ms")
end)

p.step("work", do (ctx)
  ctx["result"] = [1 to 10000].sum()
end)

p.run({})
```

### Error recovery with `on_error`

```kiwi
p = pipeline::build("resilient")

p.on_error(do (name, err, ctx)
  msg = error::message_of(err)
  println("step '${name}' failed: ${msg} — skipping")
end)

p.step("ok",   do (ctx) => ctx["a"] = 1 end)
p.step("boom", do (ctx) => throw error::ValueError("bad input") end)
p.step("also_ok", do (ctx) => ctx["b"] = 2 end)

result = p.run({})
# step 'boom' failed: bad input — skipping
# result["a"] == 1, result["b"] == 2
```
