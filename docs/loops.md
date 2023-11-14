# Loops in USL

There are a handful of ways to loop so far.

1. `for`
2. `loop`
3. `while`

### `for`

Loop on a range. Iterator symbol is set to `i`.
```
for i in (1..10)
    say "${i}"
endfor
```

Loop with default iterator symbol `$`.
```
for 1 < 10
    say "${$}"
endfor
```

Loop indefinitely.
```
for inf
    say "Press Ctrl+C to exit."
endfor
```

```
for var in 

for @n < @limit
```

