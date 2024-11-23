# `ffi`

The `ffi` (Foreign Function Interface) package provides a bridge for calling external C/C++ functions directly from Kiwi. This enables developers to extend functionality using libraries written in other languages, leveraging native performance and interoperability.

## **Package Overview**

The `ffi` package includes built-in functions for:

1. **Loading and unloading shared libraries.**
2. **Attaching functions from these libraries to the Kiwi runtime.**
3. **Invoking attached functions with dynamic parameters.**


## **Function Details**

### `attach`
```kiwi
ffi::attach(lib_alias: string, func_alias: string, 
    ffi_name: string, ffi_parameter_types: list, 
    ffi_return_type: string): boolean
```

Attaches an external function from a loaded library to the Kiwi runtime. This allows the function to be called by an alias in Kiwi.

#### **Parameters:**
- **`lib_alias`**: The alias of the library (set during library loading) containing the function.
- **`func_alias`**: The alias to use when calling the function in Kiwi.
- **`ffi_name`**: The actual name of the function in the shared library.
- **`ffi_parameter_types`**: A list of strings specifying the function's parameter types.
    - **Supported types**:
        - `int`
        - `size_t`
        - `double`
        - `bool`
        - `pointer`
        - `string`
        - `string[]`
- **`ffi_return_type`**: A string specifying the function's return type.
    - **Supported types**:
        - `int`
        - `size_t`
        - `double`
        - `bool`
        - `pointer`
        - `string`
        - `void` (Use for functions that return no value)

#### **Returns:**
- The returns a boolean indicating success or failure to attach external library function.

#### **Example:**
```kiwi
/# 
register the `c_add` function in the `mathlib` library to a function called `add` which accepts two integer parameters and returns an integer value.
#/
ffi::attach("mathlib", "add", "c_add", ["int", "int"], "int")
```

---

### `invoke`
Invokes an attached external function with a list of parameters.

#### **Syntax:**
```kiwi
ffi::invoke(func_alias: string, func_params: list): any
```

#### **Parameters:**
- **`func_alias`**: The alias of the attached function to call.
- **`func_params`**: A list of parameters to pass to the function.

#### **Returns:**
- The return value of the external function.

#### **Example:**
```kiwi
# call the `add` function we attached earlier.
result = ffi::invoke("add", [5, 7])
println result  # Output: 12
```

---

### `load`
Loads a shared library (.so) into the Kiwi runtime and assigns it an alias for future references.

#### **Syntax:**
```kiwi
ffi::load(lib_alias: string, lib_path: string)
```

#### **Parameters:**
- **`lib_alias`**: The alias to assign to the loaded library.
- **`lib_path`**: The path to the shared library file.

#### **Example:**
```kiwi
/# 
load an external shared library `libmath.so` and alias it as `mathlib`.
#/
ffi::load("mathlib", "./path/to/libmath.so")
```

---

### `unload`
Unloads a previously loaded shared library from the Kiwi runtime.

#### **Syntax:**
```kiwi
ffi::unload(lib_alias: string)
```

#### **Parameters:**
- **`lib_alias`**: The alias of the library to unload.

#### **Example:**
```kiwi
# unload the library after we are done with it.
ffi::unload("mathlib")
```

---

## **Example Usage**

[Click here](../../examples/ffi/) to view the source code.
```kiwi
/# 
Load the shared library, aliased as 'x', and attach the following functions from the library:

| return type  | function name       | function parameters        |
| ------------ | ------------------- | -------------------------- |
| const char*  | get_version         | ()                         |
| bool         | is_even             | (int* ptr)                 |
| int          | boolean_to_int      | (bool b)                   |
| int*         | create_integer      | (int value)                |
| void         | modify_integer      | (int* ptr, int new_value)  |
| int          | read_integer        | (const int* ptr)           |
| void         | free_integer        | (int* ptr)                 |
#/

ffi::load("x", "sharedlib/libexample.so")
ffi::attach("x", "version", "get_version", [], "string")
ffi::attach("x", "is_even", "is_even", ["pointer"], "bool")
ffi::attach("x", "bool2int", "boolean_to_int", ["bool"], "int")
ffi::attach("x", "create", "create_integer", ["int"], "pointer")
ffi::attach("x", "modify", "modify_integer", ["pointer", "int"], "void")  
ffi::attach("x", "read", "read_integer", ["pointer"], "int")  
ffi::attach("x", "free", "free_integer", ["pointer"], "void")

println "Version: ${ffi::invoke("version")}"

# create a pointer to an integer, set the value to 500, and then print the value the pointer points to.
xint = ffi::invoke("create", [500])
println "Value is: ${ffi::invoke("read", [xint])}"
println "Is even?: ${ffi::invoke("is_even", [xint])}"

# change the value it points to, then print the new value.
ffi::invoke("modify", [xint, 601])
println "New value is: ${ffi::invoke("read", [xint])}"
println "Is even?: ${ffi::invoke("is_even", [xint])}"

# free the pointer since we no longer need it, and unload the library!
ffi::invoke("free", [xint])

# testing boolean parameters
println "`true` converted to an integer: ${ffi::invoke("bool2int", [true])}"
println "`false` converted to an integer: ${ffi::invoke("bool2int", [false])}"

ffi::unload("x")
```

---

## **Important Notes**
1. **Error Handling**: Ensure robust error handling when working with shared libraries to avoid runtime crashes.
2. **Signatures**: Use correct FFI signatures to match the function's actual parameter and return types.
3. **Platform-Specific Paths**: Be mindful of differences in shared library paths and file extensions across operating systems.