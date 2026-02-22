### Docstrings

Docstrings are a way to add documentation to your code.

#### 1. **Syntax Basics**
- Use multi-line comments starting with `/#` and ending with `#/`. This is Kiwi's native multi-line (block) comment syntax.
- Docstrings should immediately precede the item they document (e.g., package, struct, function, or constant).
- Keep docstrings concise but informative. Aim for 1-3 sentences in the summary, with details in structured tags.
- Indentation: Align content with the opening `/#` (typically 2 spaces per level if nested).
- Line length: Prefer under 80 characters for readability.
- Markdown-like formatting is optional but encouraged for emphasis.
    - **bold**
    - *italic*
    - `code`
    - `-` (for lists)

#### 2. **Placement**
- **Package/Module Level**: At the top of the file, before any code.
- **Struct Level**: Before the `struct` declaration.
- **Function/Method Level**: Before the `fn` declaration.
- **Constant Level**: Before the `const` declaration.
- **Inline/Variable**: Rare, but possible before variable declarations if needed.

#### 3. **Required and Optional Tags**
Use `@tag` syntax for structured metadata. Tags start with `@` and are followed by a space-separated description. Multi-line tags are allowed.

- **@summary** (Required for non-trivial items): A brief one-sentence description of the item's purpose.
    - **Format:** `@summary Description`.
- **@param** (Optional, for functions/methods)
    - Describes each parameter. 
    - List one per parameter, in declaration order.
    - Use `any` for dynamic types.
    - **Format:** `@param Type name : Description`.
- **@return** (Optional, for functions/methods)
    - Describes the return value.
    - Use `any` for dynamic types.
    - **Format:** `@return Type : Description`.
- **@throws** (Optional)
    - Lists potential errors or exceptions. 
    - **Format:** `@throws ErrorType : Description`.
- **@example** (Optional)
    - Provides a usage example.
    - Use code blocks with triple dashes (`---`)
- **@see** (Optional)
    - References related items. 
    - **Format:** `@see other::function : Brief note`.
- **@deprecated** (Optional)
    - Marks as obsolete. 
    - **Format:** `@deprecated : Use alternative instead`.
- **@author** or **@version** (Rare, for packages)
    - Metadata like `@author Name` or `@version 1.0`.

#### 4. **Examples**

##### Package Docstring
```kiwi
/#
@summary A package for working with strings, including iterators and utility functions.
@version 1.3.7
@see list.kiwi : For complementary list operations.
#/
package string
  ...
end
```

##### Struct Docstring
```kiwi
/#
@summary An iterator for traversing and manipulating strings character by character.
@see string::chars() : Source of the character list.
#/
struct StringIterator
  /#
  @summary Initializes the iterator with a string.
  @param string s : The input string to iterate over.
  @return StringIterator : A new iterator instance.
  #/
  fn new(s: string)
    @chars = s.chars()
    ...
  end
  ...
end
```

##### Function Docstring
```kiwi
/#
@summary Converts all characters in the string to lowercase.
@param string _input : The input string to convert.
@return string : The lowercase version of the input.
@example
---
println(string::tolower("Hello, World!"))  # Outputs: "hello, world!"
---
#/
fn tolower(_input)
  return ("${_input}").lowercase()
end
```

##### Constant Docstring
```kiwi
/#
@summary The version of the Kiwi standard library.
@deprecated : Use env::get("KIWI_VERSION") instead.
#/
const KIWI_STDLIB_VERSION = "1.3.7"
```

#### 5. **Best Practices**
- **Consistency**: Always use `@summary` for discoverability. Omit tags if irrelevant (e.g., no params means no `@param`).
- **Types**: Use Kiwi type names (e.g., `string`, `integer`, `list`, `hashmap`, `any`, `boolean`). For custom types, reference them (e.g., `Path`).
- **Errors/Exceptions**: If a function can `throw`, document it with `@throws`.
- **Examples**: Keep them short and executable. Use `#` for comments in examples.
- **Tools**: This format is parseable for auto-generating docs (e.g., via a future Kiwi tool or script using `string` and `list` packages to extract tags).
- **Avoid Redundancy**: Don't repeat the function name or params in the summary—focus on *what* and *why*, not *how*.
- **Localization**: Docstrings are in English by default, but tags can be extended for other languages if needed.