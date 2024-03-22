# REPL

### Introduction to the REPL

To use the REPL, run Astral without passing any arguments. 

Astral will print its current version (at the time of writing this, the current version is 1.3), provide some instructions and then a prompt line beginning with **`> `**.

```
$ astral
The Astral Programming Language v1.3.2 REPL

Use `go` to execute, `exit` to exit the REPL.

>   
```

The Astral REPL enqueues code as you enter it. It also keeps track of all lines you enter so that you can replay them.

Type some Astral code into the REPL prompt and press <kbd>Enter</kbd>.

```
> println "Hello, World!" 
```

When you are ready to execute, type `go` and press <kbd>Enter</kbd>. Astral will execute the code in the queue and then reset the queue.

```
> println "Hello, World!" 
> go
Hello, World!
> 
```

### Keyboard Controls

You can use the <kbd>&#8592;</kbd> and <kbd>&#8594;</kbd> arrow keys to navigate to different parts of the line for inline editing of the code before submitting to the REPL queue.

You can use the <kbd>&#8593;</kbd> and <kbd>&#8595;</kbd> arrow keys to cycle through the list of lines entered.