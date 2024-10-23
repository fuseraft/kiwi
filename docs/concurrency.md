# Concurrency

## `async`

You can define an asynchronous method with the `async` keyword.

```kiwi
async fn long_runner(sleep_ms)
  __delay__(sleep_ms) # simulate work
  return 42 # some work result
end
```

## `await`

Use the `await` keyword to invoke an asynchronous method and store the result.

```kiwi
result = await long_runner(1000) # sleep for 1 second.
println(result) # prints: 42

await long_runner(500) # sleep for 0.5 seconds.
println("done!") # prints: done!
```

## `then`

Use the `then` keyword to invoke an asynchronous method and specify logic to execute after completion.

```kiwi
long_runner(500) then (i) do
  println("The answer is ${i}") # prints: The answer is 42.
end
```

The syntax for the `then` lambda is:

```
then (result [, task_id]) do
  statements
  ...
end
```