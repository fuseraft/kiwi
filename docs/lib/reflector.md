# `reflector`

The `reflector` package contains functionality for reflection in Kiwi.

## Table of Contents

- [Package Functions](#package-functions)
  - [`callstack()`](#callstack)
  - [`fflags(frame_index)`](#fflagsframe_index)
  - [`getfunc(func_name)`](#getfuncfunc_name)
  - [`objcontext()`](#objcontext)
  - [`retval()`](#retval)
  - [`state()`](#state)
- [`Callable` struct](#callable)
  - [Constructor](#constructor)
  - [`from_function(func_name)`](#from_functionfunc_name)
  - [`call(params)`](#callparams)
  - [`parameters()`](#parameters)
  - [`returns()`](#returns)

## Package Functions

### `callstack()`
Returns a list of frame names.

**Parameters**  
_None_

**Returns**  
| Type | Description |  
| :--- | :--- |  
| `list` | A list containing the names of each stack frame in the callstack. |

---

### `fflags(frame_index)`
Returns a list of frame flags at a specified index.

**Parameters**  
| Type | Name | Description |  
| :--- | :--- | :--- |  
| `integer` | `frame_index` | The frame index. Defaults to 0 (top of the stack). |

**Returns**  
| Type | Description |  
| :--- | :--- |  
| `list` | The frame flags at specific frame index. |

---

### `getfunc(func_name)`
Returns a specified function as a lambda.

**Parameters**  
| Type | Name | Description |  
| :--- | :--- | :--- |  
| `string` | `func_name` | The name of the function. |

**Returns**  
| Type | Description |  
| :--- | :--- |  
| `any` | The function as a lambda. Returns `null` when not found. |

---

### `objcontext()`
Returns a hashmap representing the state of the program.

**Parameters**  
_None_

**Returns**  
| Type | Description |  
| :--- | :--- |  
| `any` | Returns the current object reference. Returns `null` when not in an object context. |

---

### `retval()`
Returns the current stack frame return value.

**Parameters**  
_None_

**Returns**  
| Type | Description |  
| :--- | :--- |  
| `any` | The return value at the top of the stack. |

---

### `state()`
Returns a hashmap representing the state of the program.

**Parameters**  
_None_

**Returns**  
| Type | Description |  
| :--- | :--- |  
| `hashmap` | A hashmap containing four keys: `packages` (a list containing names of packages imported), `structs` (a list containing names of structs defined), `functions` (a list containing names of functions defined), `stack` (a list of hashmaps containing frame variables, each with `variables` as a hashmap of variable names to values). |

---

## `Callable`

A struct representing a callable function or lambda, providing reflection capabilities such as invoking it and inspecting its parameters and return type.

### Constructor

Creates a new Callable instance.

**Parameters**  
| Type | Name | Description |  
| :--- | :--- | :--- |  
| `string` | `name` | The name of the callable. |  
| `lambda` | `func` | The lambda representing the callable. |

### `from_function(func_name)`

Static method to create a Callable from a function name.

**Parameters**  
| Type | Name | Description |  
| :--- | :--- | :--- |  
| `string` | `func_name` | The name of the function. |

**Returns**  
| Type | Description |  
| :--- | :--- |  
| `Callable` | A new Callable instance wrapping the function. |

### `call(params)`

Invokes the callable with a set of parameters and returns its value.

**Parameters**  
| Type | Name | Description |  
| :--- | :--- | :--- |  
| `list` | `params` | A list of parameters to pass to the callable. |

**Returns**  
| Type | Description |  
| :--- | :--- |  
| `any` | The return value of the callable after invocation. |

### `parameters()`

Returns a list of hashmaps containing parameter information.

**Parameters**  
_None_

**Returns**  
| Type | Description |  
| :--- | :--- |  
| `list` | A list of hashmaps, each containing: `name` (a string representing the name of the parameter), `default_value` (the default value), `type` (if a type annotation was used, the type name as a string), `position` (the integer position of the parameter). |

### `returns()`

Returns the type name of the return type of the callable.

**Parameters**  
_None_

**Returns**  
| Type | Description |  
| :--- | :--- |  
| `string` | A string containing the type name of the return type. |