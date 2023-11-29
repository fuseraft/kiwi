# Loops

There are a handful of ways to loop so far.

1. `for`
2. `loop`
3. `while`

### `for`

Loop on a range. Iterator symbol is set to `i`.
```
for i in (1..10)
    println "${i}"
endfor
```

Loop with default iterator symbol `$`.
```
for 1 < 10
    println "${$}"
endfor

@start = 0
@stop = 10
for @start < @stop
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

