# pipeline

> Named, step-by-step data pipelines with hooks and error recovery.

The `pipeline` package runs a sequence of named steps over a shared context hashmap. Each step reads from and writes to the context, which keeps multi-stage scripts (load, clean, transform, report) easy to follow and to instrument.

```kiwi
etl = pipeline::build("people")

etl.step("load", do (ctx)
  ctx["rows"] = [
    {name: "Alice", age: 32, active: true},
    {name: "Bob", age: 17, active: true},
    {name: "Carol", age: 25, active: false}
  ]
end)

etl.step("filter", do (ctx)
  ctx["rows"] = ctx["rows"].filter(do (r) => r["active"] && r["age"] >= 18 end)
end)

etl.step("names", do (ctx)
  ctx["names"] = ctx["rows"].map(do (r) => r["name"] end)
end)

result = etl.run({})
println result["names"]   # ["Alice"]
```

`run(ctx)` executes the steps in the order they were added and returns the context.

## Hooks

`before_each` and `after_each` run around every step and receive the step name and the context:

```kiwi
timed = pipeline::build("timed")

timed.before_each(do (name, ctx)
  ctx["_started"] = time::ticks()
end)

timed.after_each(do (name, ctx)
  elapsed = time::ticksms(time::ticks() - ctx["_started"])
  println "${name} finished"
end)

timed.step("sum", do (ctx)
  ctx["total"] = [1 to 10000].sum()
end)

println timed.run({})["total"]   # 50005000
```

## Errors

Without an error handler, a step that throws stops the pipeline and the error propagates out of `run`. With `on_error`, the handler receives the step name, the error and the context, and the pipeline continues with the next step:

```kiwi
resilient = pipeline::build("resilient")

resilient.on_error(do (name, err, ctx)
  println "step '${name}' failed: ${error::message_of(err)}"
end)

resilient.step("first", do (ctx) => ctx["a"] = 1 end)
resilient.step("broken", do (ctx) => throw error::ValueError("bad input") end)
resilient.step("last", do (ctx) => ctx["b"] = 2 end)

result = resilient.run({})
println [result["a"], result["b"]]   # [1, 2]
```

## Functions

### `pipeline::build`

```kiwi
pipeline::build(name: string): Pipeline
```

Creates and returns a new Pipeline.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | A label for the pipeline. |

**Returns** `Pipeline` — A new empty pipeline.

## `Step` struct

Represents a single named step in a pipeline.

### `Step.new`

```kiwi
Step.new(name: string, action: lambda)
```

Creates a new step.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The display name of the step. |
| `action` | `lambda` |  | A lambda accepting the context hashmap. |

### `.run`

```kiwi
run(ctx: hashmap)
```

Executes the step action with the given context.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ctx` | `hashmap` |  | The shared pipeline context. |

## `Pipeline` struct

A named sequence of steps that share a context hashmap.

### `Pipeline.new`

```kiwi
Pipeline.new(name: string)
```

Creates a new pipeline.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | A label for this pipeline (used in output and errors). |

### `.step`

```kiwi
step(name: string, action: lambda)
```

Appends a named step to the pipeline.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The display name of the step. |
| `action` | `lambda` |  | A lambda accepting the context hashmap. |

### `.before_each`

```kiwi
before_each(hook: lambda)
```

Registers a hook called before each step.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `hook` | `lambda` |  | A lambda accepting (step_name: string, ctx: hashmap). |

### `.after_each`

```kiwi
after_each(hook: lambda)
```

Registers a hook called after each successful step.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `hook` | `lambda` |  | A lambda accepting (step_name: string, ctx: hashmap). |

### `.on_error`

```kiwi
on_error(handler: lambda)
```

Registers an error handler called when a step throws.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `handler` | `lambda` |  | A lambda accepting (step_name, err, ctx). |

### `.run`

```kiwi
run(ctx: hashmap = {})
```

Runs all steps in order, passing the context through each.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ctx` | `hashmap` | `{}` | The initial context. Defaults to an empty hashmap. |

**Returns** `hashmap` — The context after all steps have run.
