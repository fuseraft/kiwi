# Kiwi FFI Package Documentation

The `ffi` (Foreign Function Interface) package provides a bridge for calling external C/C++ functions directly from Kiwi. This enables developers to extend functionality using libraries written in other languages, leveraging native performance and interoperability.

## **Package Overview**

The `ffi` package includes built-in functions for:

1. **Loading and unloading shared libraries.**
2. **Attaching functions from these libraries to the Kiwi runtime.**
3. **Invoking attached functions with dynamic parameters.**


## **Function Details**

### `attach`

#### **Description:**
Attaches an external function from a loaded library to the Kiwi runtime. This allows the function to be called by an alias in Kiwi.

#### **Syntax:**
```kiwi
ffi::attach(lib_alias: string, func_alias: string, ffi_name: string, ffi_signature: string)
```

#### **Parameters:**
- **`lib_alias`**: The alias of the library (set during library loading) containing the function.
- **`func_alias`**: The alias to use when calling the function in Kiwi.
- **`ffi_name`**: The actual name of the function in the shared library.
- **`ffi_signature`**: A string specifying the function's parameter and return types.

#### **Example:**
```kiwi
ffi::attach("mathlib", "add", "c_add", "int,int -> int")
```

---

### `invoke`

#### **Description:**
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
result = ffi::invoke("add", [5, 7])
println(result)  # Output: 12
```

---

### `load`

#### **Description:**
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
ffi::load("mathlib", "/usr/lib/libmath.so")
```

---

### `unload`

#### **Description:**
Unloads a previously loaded shared library from the Kiwi runtime.

#### **Syntax:**
```kiwi
ffi::unload(lib_alias: string)
```

#### **Parameters:**
- **`lib_alias`**: The alias of the library to unload.

#### **Example:**
```kiwi
ffi::unload("mathlib")
```

---

## **Example Usage**

### **Complete Example**
```kiwi
# Load the shared library
ffi::load("mathlib", "/usr/lib/libmath.so")

# Attach a function from the library
ffi::attach("mathlib", "add", "c_add", "int,int,int")

# Call the function
sum = ffi::invoke("add", [10, 20])
println(sum)  # Output: 30

# Unload the library when done
ffi::unload("mathlib")
```

---

## **Important Notes**
1. **Error Handling**: Ensure robust error handling when working with shared libraries to avoid runtime crashes.
2. **Signatures**: Use correct FFI signatures to match the function's actual parameter and return types.
3. **Platform-Specific Paths**: Be mindful of differences in shared library paths and file extensions across operating systems.