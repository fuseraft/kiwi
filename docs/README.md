# Astral Index

1. **Getting Started**
   - [Using the Astral CLI](cli.md)

2. **Language Basics**
   - [Comments](comments.md)
   - [Types](types.md)
   - [Operators](operators.md)
   - [Keywords](keywords.md)
   - [Variables](variables.md)
   - [Methods](methods.md)
   - [Lambdas](lambdas.md)

3. **Advanced Language Features**
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

4. **Standard Library**
   - [`@astral/argv`](lib/argv.md)
   - [`@astral/conf`](lib/conf.md)
   - [`@astral/env`](lib/env.md)
   - [`@astral/fs`](lib/fs.md)
   - [`@astral/log`](lib/log.md)
   - [`@astral/math`](lib/math.md)
   - [`@astral/string`](lib/string.md)
   - [`@astral/sys`](lib/sys.md)
   - [`@astral/time`](lib/time.md)
     - [`DateTime`](lib/datetime.md)
   - [`@astral/web`](lib/web.md)
   - [`@astral/http`](lib/http.md)

5. **Experimental Features**
   - [Concurrency](concurrency.md)

# Astral In-a-Nutshell

When you execute Astral, a number of things happens.

A series of steps happen before your program runs, 

## Pre-Execution

### Host

Astral runs within a *host* that is responsible for registering command-line arguments, loading libraries, and running your program.

#### Hosting

1. Arguments passed to the program are registered.
2. The **Standard** and **Local** libraries are loaded.
    1. **Standard**
        1. The *standard library* exists relative to Astral.
        2. Astral finds its executable location, then searches the parent directory for a subdirectory called `lib`.
            1. If Astral is located at `/usr/bin/astral`, then the Standard Library must be located at `/usr/lib/astral`.
    2. **Local**
        1. A *local library* exists relative to your program.
        2. Astral searches for a subdirectory called `lib` in the same directory as your program.
3. Your program code is passed to the *lexer*.

### Lexer

Before source code is interpreted, it is sent to the lexer to be *tokenized* into a *token stream*.

#### Tokenization

1. The file is registered into the *file registry*.
    1. The file content is stored in the registry.
    2. The registry generates a *file identifier* which links a token to the file.
2. The file content is tokenized into a *token stream*.
    1. Each token is assigned a *file identifier*, *row*, *column*, *type*, *name*, *text*, and *value*.
        1. The *file identifier*, *row*, and *column* are used for tracing.
        2. The *type* and *name* are used to categories the token.
        3. The *text* contains the token text.
        4. The *value* contains the parsed value for string, numeric, or boolean literals.

#### Token Stream

The tokens are encapsulated into a *token stream*.

The token stream is an iterator consumed by the interpreter.

### Interpreter

The *interpreter* processes the token stream and executes code on the fly.

#### Call Stack

The *call stack* is used to manage program execution in executable units called *frames*. The interpreter runs the code at the top of the stack and as new frames are added to the stack, the interpreter executes them in sequence.

#### Frames

The *frame* is used to control scope at runtime. Each frame has its own *state*. Some parts of the frame state are propagated to other frames when required.

#### Frame State

The frame *state* contains information about the frame including, frame *flags*, locally-scoped *references*, *context*, and *error state*.

Frame *flags* are bit-flags the interpreter uses to process the token stream. For example, when processing a loop, a *sub-frame* is created with a flag indicating it is being executed in a loop. If within the loop frame, a `break` instruction is processed, then a flag will be set on the frame to indicate it is time to end the loop. 

References created within the scope of the executing frame can be passed to sub-frames (e.g. method arguments, outer-scoped variables accessed within a loop).

Some state is passed down the call stack. For example, when the `return` instruction is processed within a method call, if a return value is passed, then the calling frame will receive that return value.

The frame *context* is *global* by default. If a frame is executing within an object instance, then the context is specific to the instance. This is important for the interpreter to support object-oriented programming.

In the event of an exception, the *error state* is set. The error state contains the type of error, the message, and the area in the code where the exception occurred. 

In the event of an uncaught exception, the error handler searches the file registry for the file identifier associated with the token, retrieves the line from the file where the error occurred, and prints the error type, message, the line of code, and a pointer to the specific token.

At the end of execution, the last frame is popped from the callstack and the program exits.