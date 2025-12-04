# REPL

To use the REPL, run `kiwi -i` or `kiwi --interactive`.

```
$ kiwi -i
>   
```

Try playing around with Kiwi by typing some code into the REPL prompt and press <kbd>Enter</kbd>.

```
> println "Hello, World!"
Hello, World!
```

To write a block of code, end your lines with `\`.

```
> for v, i in ["hello", "world"] do \
>   println "${i}: ${v}" \
> end
0: hello
1: world
```

To end the session, just press <kbd>Ctrl+C</kbd> or type `exit` and press <kbd>Enter</kbd>.