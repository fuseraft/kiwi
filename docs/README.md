# Kiwi Index

1. **Getting Started**
   - [Using the Kiwi CLI](cli.md)

2. **Language Basics**
   - [Comments](comments.md)
   - [Types](types.md)
   - [Operators](operators.md)
   - [Keywords](keywords.md)
   - [Variables](variables.md)
   - [Methods](methods.md)
   - [Lambdas](lambdas.md)

3. [**Life of a Program**](#life-of-a-program)

4. **Advanced Language Features**
   - [Modules](modules.md)
   - [Error Handling](error_handling.md)
   - **Control Flow**
     - [Conditionals](conditionals.md)
     - [Loops](loops.md)
   - **Collections**
     - [Lists](lists.md)
     - [Ranges](ranges.md)
     - [Hashes](hashes.md)
   - **Builtins**
     - [Builtins](builtins.md)
     - [Console I/O](console_io.md)
   - **Object-Oriented Programming**
     - [Classes](classes.md)
     - [Abstract Classes](abstract_classes.md)

5. **Standard Library**
   - [`@kiwi/argv`](lib/argv.md)
   - [`@kiwi/conf`](lib/conf.md)
   - [`@kiwi/env`](lib/env.md)
   - [`@kiwi/fs`](lib/fs.md)
   - [`@kiwi/log`](lib/log.md)
   - [`@kiwi/math`](lib/math.md)
   - [`@kiwi/string`](lib/string.md)
   - [`@kiwi/sys`](lib/sys.md)
   - [`@kiwi/time`](lib/time.md)
     - [`DateTime`](lib/datetime.md)
   - [`@kiwi/web`](lib/web.md)
   - [`@kiwi/http`](lib/http.md)

6. **Experimental Features**
   - [Concurrency](concurrency.md)

# Life of a Program

When you execute Kiwi, a number of things happens.

Below is a technical overview of the architecture of the interpreter.

## Pre-Execution

Kiwi will configure its runtime environment before executing your program. Below is a high-level overview of the components involved in this process.

### Host

Kiwi runs within a *host* that is responsible for registering command-line arguments, loading libraries, and running your program.

Arguments passed to the program are registered.

The **Standard** and **Local** libraries are loaded.

#### Standard Library

The *standard library* exists relative to Kiwi.

Kiwi finds its executable location, then searches the parent directory for a subdirectory called `lib`.

If Kiwi is located at `/usr/bin/kiwi`, then the Standard Library must be located at `/usr/lib/kiwi`.

#### Local Library

A *local library* exists relative to your program.

Kiwi searches for a subdirectory called `lib` in the same directory as your program.

### Lexer

Before source code is interpreted, it is sent to the lexer to be *tokenized* into a *token stream*.

#### Tokenization

The file is registered into the *file registry*.
1. The file content is stored in the registry.
2. The registry generates a *file identifier* which links a token to the file.

The file content is tokenized into a *token stream*.
1. Each token is assigned a *file identifier*, *row*, *column*, *type*, *name*, *text*, and *value*.
   1. The *file identifier*, *row*, and *column* are used for tracing.
   2. The *type* and *name* are used to categorize the token.
   3. The *text* contains the token text.
   4. The *value* contains the parsed value for string, numeric, or boolean literals.

#### Token Stream

The tokens are encapsulated into a *token stream*.

The token stream is an iterator consumed by the interpreter.

## Execution

Now it's time to run your program.

### Interpreter

The *interpreter* processes token streams and executes code on the fly.

#### Call Stack

The *call stack* is used to manage program execution in executable units called *frames*. The interpreter runs the code at the top of the stack and as new frames are added to the stack, the interpreter executes them in sequence.

#### Frames

The *frame* is used to control scope at runtime. Each frame has its own *state*. Some parts of the frame state are propagated to other frames when required.

#### Frame State

The frame *state* contains information about the frame including, frame *flags*, locally-scoped *references*, *context*, and *error state*.

Frame *flags* are bit-flags the interpreter uses to process the token stream. 

For example, when processing a loop, a *sub-frame* is created with a flag indicating it is being executed in a loop. 

```ruby
# define a loop that runs 10 times.
for i in [1 .. 10] do
  # The loop frame begins here.
  println("I have ran ${i} time${i > 1 ? "s" : ""}.")
  # The loop frame here.
end

/# 
Output:
I have ran 1 time.
I have ran 2 times.
I have ran 3 times.
I have ran 4 times.
I have ran 5 times.
I have ran 6 times.
I have ran 7 times.
I have ran 8 times.
I have ran 9 times.
I have ran 10 times.
#/
```

If within the loop frame, a `break` instruction is processed, then a flag will be set on the frame to indicate it is time to end the loop. 

```ruby
# define a loop that runs 10 times.
for i in [1 .. 10] do
  # The loop frame begins here.
  println("I have ran ${i} time${i > 1 ? "s" : ""}.")

  if i == 1
    # sub-frame begins here if the condition `i == 1` evaluates to `true`.
    break # sets the loop-break flag.
    # sub-frame ends here.
  end

  # The loop frame ends here.
end

/# 
Output:
I have ran 1 time.
#/
```

Some state is passed down the call stack. For example, when the `return` instruction is processed within a method call, if a return value is passed, then the calling frame will receive that return value.

```ruby
# The program frame begins here.
def add(n, m)
  # The sub-frame begins here.
  return n + m # The return flag is set and the value is propagated to the calling frame.
  # The sub-frame ends here.
end

# Call `add()` and assign its return value to a variable called `number`.
number = add(3, 6)
println(number)   # Prints: 9
# The program frame ends here.
```

References created within the scope of the executing frame can be passed to sub-frames (e.g. method arguments, outer-scoped variables accessed within a loop).

```ruby
# The program frame begins here.

# The `update` method sets the value of `variable` to the value of `value`. 
def update(variable, value)
  # The sub-frame begins here.
  variable = value
  # The sub-frame ends here.
end

/# 
The `set_value` method calls the `update` method, passing a reference to `variable` and a reference to `value`.
It then prints the value of the variable after the call.
#/
def set_value(variable, value)
  # The sub-frame begins here.
  update(variable, value)        # Call `update()`, passing references to `variable` and `value`.
  return variable                # Return the value of variable (to be printed).
  # The sub-frame ends here.
end

# A variable called `number` is set to 0.
number = 0

println(number)                  # Prints: 0
println(set_value(number, 1))    # Prints: 1
println(number)                  # Prints: 0

# The program frame ends here.
```

The frame *context* is *global* by default. If a frame is executing within an object instance, then the context is specific to the instance. This is important for the interpreter to support object-oriented programming.

In the event of an exception, the *error state* is set. The error state contains the type of error, the message, and the area in the code where the exception occurred. 

In the event of an uncaught exception, the error handler searches the file registry for the file identifier associated with the token, retrieves the line from the file where the error occurred, and prints the error type, message, the line of code, and a pointer to the specific token.

At the end of execution, the last frame is popped from the callstack and the program exits.