# Loops

*Under construction in new interpreter. Only `while` is supported for now*

There are a handful of ways to loop so far.

1. `for`
2. `loop`
3. `while`

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

Loop on a range. Iterator symbol is set to `i`.
```
for i in (1..10)
    println "${i}"
endfor
```

Loop with default iterator symbol `$`.
```
for 1 to 10
    println "${$}"
endfor

@start = 0
@stop = 10
for @start to @stop
    println "${$}"
endfor
```

Loop indefinitely.
```
for inf
    println "Press Ctrl+C to exit."
endfor
```

Loop command-line arguments.
```
for each in args.values
    println "${$}"
endfor
```

