# Loops

*Under construction in new interpreter. Only `while` is supported for now*

There are a handful of ways to loop so far.

1. `while`
2. `for`

### `while`

Loop based on a condition. *See the test script for a better example with nesting.*
```
@i = 0
while @i < 10 do
    @i += 1
end

println @i
```

### `for`

Loop on a range. Iterator symbol is set to `@i`.
```
for @i in (1..10)
    println "${@i}"
endfor
```

Loop indefinitely.
```
while true
    println "Press Ctrl+C to exit."
end
```


