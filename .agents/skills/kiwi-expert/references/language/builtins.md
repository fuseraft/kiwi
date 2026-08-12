# Builtins

In Kiwi, builtins are accessed using dot-notation and can be used to query or manipulate values and types.

## `global`

The `global` variable is a hashmap that can be used to store global data. This is useful for sharing data between scripts.

## Date Builtins

### `year()`

Returns the year part.

### `month()`

Returns the month part. (1 through 12)

### `day()`

Returns the day part. (1 through number of days in the month)

### `hour()`

Returns the hour part. (0 through 23)

### `minute()`

Returns the minute part. (0 through 59)

### `second()`

Returns the second part. (0 through 59)

### `millisecond()`

Returns the millisecond part. (0 through 999)

## String Builtins

### `begins_with(str)`

Returns true if the string begins with a given string.

```kiwi
println("foobar".begins_with("foo"))   # prints: true
println("foobar".begins_with("food"))  # prints: false
```

### `chars()`

Converts a string into a list. Each character in the string becomes a new string in the list.

```kiwi
string = "Hello"
chars = string.chars()
# chars = ["H", "e", "l", "l", "o"]

println("kiwi".chars()) # prints: ["k", "i", "w", "i"]
```

### `chomp()`

Strips a trailing newline (`\r\n`, `\n`, or `\r`) from the string. Returns the string unchanged if it does not end with a newline.

```kiwi
line = "Hello, World!\n"
println line.chomp()  # prints: Hello, World!

line2 = "No newline"
println line2.chomp() # prints: No newline
```

### `contains(str)`

Returns true if the string contains a given string.

```kiwi
println("foobar".contains("bar"))   # prints: true
println("foobar".contains("bark"))  # prints: false
```

### `ends_with(str)`

Returns true if the string ends with a given string.

```kiwi
println("foobar".ends_with("bar"))   # prints: true
println("foobar".ends_with("bark"))  # prints: false
```

### `hex_bytes()`

Parses a hex-encoded string into a bytes value. The string may contain spaces and tabs (which are stripped). The hex string must have an even number of characters after whitespace is removed.

```kiwi
data = "48 65 6C 6C 6F".hex_bytes()
println data.size()  # prints: 5
println data         # prints the raw byte array
```

### `index(str)`

Returns the index of a string. Returns -1 if not found.

```kiwi
println("foobar".index("bar"))    # prints: 3
println("foobar".index("kiwi"))   # prints: -1
```

### `lastindex(str)`

Returns the last index of a string. Returns -1 if not found.

```kiwi
println("foobarbar".lastindex("bar"))  # prints: 6
println("foobar".lastindex("kiwi"))    # prints: -1
```

### `lines()`

Splits the string into a list of lines, splitting on the platform newline sequence.

```kiwi
text = "line one\nline two\nline three"
println text.lines()  # prints: ["line one", "line two", "line three"]
```

### `lowercase()`

Returns the lowercase value of a string.

```kiwi
println("FOOBAR".lowercase())  # prints: foobar
```

### `ltrim()`

Trims whitespace from the left-hand side of a string.

```kiwi
println("   Hello World!".ltrim()) # prints: Hello World!
```

### `ord()`

Returns the Unicode code point (as an integer) of the first character of the string. Throws an error if the string is empty.

```kiwi
println("A".ord())    # prints: 65
println("kiwi".ord()) # prints: 107
println("🥝".ord())   # prints: 129373
```

### `rtrim()`

Trims whitespace from the right-hand side of a string.

```kiwi
println("Hello World!    ".rtrim() + " Testing!") # prints: Hello World! Testing!
```

### `trim()`

Trims whitespace from both sides of a string.

```kiwi
println("     Hello World!    ".trim() + " Testing!") # prints: Hello World! Testing!
```

### `uppercase()`

Returns the uppercase value of a string.

```kiwi
println("foobar".uppercase())  # prints: FOOBAR
```

### `substring(pos, length)`

Extract a substring from a string.

```kiwi
println("hello".substring(1))    # prints: ello
println("hello".substring(1, 2)) # prints: el
```

## Regex Builtins

### `find(regex)`

Searches for the first occurrence of a pattern described by a regex and returns the substring.

```kiwi
println("my email: example@test.com".find('\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z0-9]{2,}\b'))
# prints: example@test.com
```

### `match(regex)`

Returns the capture groups for the first match of the regex in the string.

```kiwi
println("June 24, 2021".match('(\w+) (\d+), (\d+)')) # prints: ["June", "24", "2021"]
```

### `matches(regex)`

Tests whether the entire string conforms to a regular expression pattern.

```kiwi
println("hello123".matches('^([a-z]+\d{3})$'))   # prints: true
println("hello123!".matches('^([a-z]+\d{3})$'))  # prints: false
```

### `matches_all(regex)`

Checks if all parts of the string conform to the regex pattern.

```kiwi
println("123-456-7890".matches_all('\d{3}-\d{3}-\d{4}'))  # prints: true
println("123-456-789x".matches_all('\d{3}-\d{3}-\d{4}'))  # prints: false
```

### `replace(search, replacement)`

Search for a string (or regex pattern) and replace with a given string. Supports regex capture group back-references.

```kiwi
println("foobar".replace("foo", "food"))       # prints: foodbar
println("foo123bar".replace('(\d+)', '[$1]'))  # prints: foo[123]bar
println("foo123bar456".replace('\d+', "-"))    # prints: foo-bar-
```

### `rreplace(pattern, replacement)`

Like `replace`, but replaces only the **last** occurrence of the pattern.

```kiwi
println("aabbcc".rreplace("b", "X"))         # prints: aabXcc
println("foo-bar-baz".rreplace('-\w+', "!")) # prints: foo-bar!
```

### `rsplit(delimiter, limit = -1)`

Splits a string by delimiter, working from **right to left**. The optional `limit` caps the number of splits. Useful for peeling off a suffix or file extension.

```kiwi
println("a.b.c.d".rsplit(".", 1))  # prints: ["a.b.c", "d"]
println("a.b.c.d".rsplit("."))     # prints: ["a", "b", "c", "d"]
```

### `named_captures(regex)`

Returns a hashmap mapping each named capture group to its matched value for the first match of the pattern. Returns an empty hashmap if there is no match or the pattern contains no named groups.

```kiwi
s = "John Smith, age 42"
m = s.named_captures('(?<name>\w+ \w+), age (?<age>\d+)')
println m["name"]  # prints: John Smith
println m["age"]   # prints: 42
```

### `regex_escape()`

Escapes all regex metacharacters in the string so it can be used as a literal pattern.

```kiwi
raw = "2.0 (beta)"
escaped = raw.regex_escape()
println escaped  # prints: 2\.0\ \(beta\)

# Safe to use in a pattern now:
println "version 2.0 (beta)!".matches(".*${escaped}.*")  # prints: true
```

### `scan(regex)`

Finds every occurrence of the regex in the string and returns a list of matches.

```kiwi
println("s7s s8s s9s".scan('\d'))  # prints: ["7", "8", "9"]
```

### `scan_groups(regex)`

Like `scan`, but returns **capture groups** for every match instead of the full match text. Each element in the returned list is itself a list of capture group strings for one match.

```kiwi
text = "2024-01-15, 2024-06-30"
groups = text.scan_groups('(\d{4})-(\d{2})-(\d{2})')
println groups
# prints: [["2024", "01", "15"], ["2024", "06", "30"]]
```

### `split(delim, limit = -1)`

Splits a string into a list by delimiter.

```kiwi
println("Hello World!".split(" "))           # prints: ["Hello", "World!"]
println("one,two,three,four".split(",", 2))  # prints: ["one", "two,three,four"]
```

## Range Checking

### `between(a, b)`

Returns true if the value falls within the inclusive range `[a, b]`. Works on numbers, strings (lexicographic order), and dates.

```kiwi
# Numbers
println (5).between(1, 10)    # prints: true
println (15).between(1, 10)   # prints: false

# Floats
println (3.14).between(3.0, 4.0)  # prints: true

# Strings (lexicographic order)
println "kiwi".between("apple", "mango")   # prints: true
println "zebra".between("apple", "mango")  # prints: false

# Dates
start = "2024-01-01".to_date()
end   = "2024-12-31".to_date()
today = "2024-06-15".to_date()
println today.between(start, end)  # prints: true
```

## Hashmap Builtins

### `keys()`

Returns the list of keys from a hashmap.

```kiwi
hashmap = {
  "key1": true,
  "key2": 1,
  "key3": ["a", "b", "c"]
}

println(hashmap.keys()) # prints: ["key1", "key2", "key3"]
```

### `has_key(key)`

Returns true if a hashmap contains a given key.

```kiwi
hashmap = { "key1": true, "key2": 1 }
println(hashmap.has_key("key2")) # prints: true
println(hashmap.has_key("key9")) # prints: false
```

### `get(key, default = null)`

Returns the value assigned to a given key. If the key is not present and a default is supplied, returns the default instead of `null`.

```kiwi
hashmap = { "a": 1, "b": 2 }

println(hashmap.get("a"))         # prints: 1
println(hashmap.get("z", 99))     # prints: 99
println(hashmap.get("z"))         # prints: null
```

### `set(key, value)`

Sets the value assigned to a given key (creates the key if it does not exist).

```kiwi
hashmap = { "key1": true }
hashmap.set("key1", false)
hashmap.set("key2", 42)
println hashmap  # prints: {"key1": false, "key2": 42}
```

### `merge(hashmap)`

Merge a hashmap with another. Keys in the argument override matching keys in the receiver.

```kiwi
hashmap1 = {"a": 1, "b": 2}
hashmap2 = {"b": 3, "c": 4}
println(hashmap1.merge(hashmap2)) # prints: {"a": 1, "b": 3, "c": 4}
```

### `remove(key)`

Removes a key-value pair from the hashmap. Returns the hashmap after removal.

```kiwi
h = { "a": 1, "b": 2, "c": 3 }
h.remove("b")
println h  # prints: {"a": 1, "c": 3}
```

### `values()`

Returns the list of values from a hashmap.

```kiwi
hashmap = { "key1": true, "key2": 1, "key3": ["a", "b", "c"] }
println(hashmap.values()) # prints: [true, 1, ["a", "b", "c"]]
```

## List Builtins

### `all(lambda)`

Returns true if all elements in a list match a given condition.

```kiwi
list = [2, 4, 6]
println list.all(do (n) => n % 2 == 0)  # prints: true

list.push(5)
println list.all(do (n) => n % 2 == 0)  # prints: false
```

### `append(value)`

Alias for `push`. Adds a value to the end of a list.

```kiwi
list = [1, 2, 3]
list.append(4)
println list  # prints: [1, 2, 3, 4]
```

### `clear()`

Clears a list or a hashmap.

```kiwi
list = "Hello".chars() # ["H", "e", "l", "l", "o"]
list.clear()           # []
```

### `concat(list)`

Combine two lists into one.

```kiwi
println([1, 2].concat([3, 4])) # prints: [1, 2, 3, 4]
```

### `count(value)`

Count occurrences of a specific value in the list.

```kiwi
println("hello world".chars().count("o")) # prints: 2
```

### `dequeue()`

Removes and returns a value from the beginning of a list.

```kiwi
list = [1, 2, 3]
println(list.dequeue()) # prints: 1
println(list)           # prints: [2, 3]
```

### `each(lambda)`

Iterate a list, performing some action for each item in the list.

```kiwi
# Convert "hello" to a list of unique values, and iterate each.
"hello".chars().unique().each(do (v, i) => println "${i} = ${v}")

/# Prints:
0 = h
1 = e
2 = l
3 = o
#/

# Iterate a range.
[1 to 5].each(do (v, i) => println "${i}: ${v}")

/# Prints:
0: 1
1: 2
2: 3
3: 4
4: 5
#/
```

### `enqueue(value)`

Pushes a value onto the end of a list (alias for `push`).

```kiwi
list = [1, 2, 3]
list.enqueue(4)
println list  # prints: [1, 2, 3, 4]
```

### `filter(lambda)`

Filter a list based on a condition.

```kiwi
words = ["kiwi", "mango", "strawberry"]
println words.filter(do (w) => w.size() > 4)
# prints: ["mango", "strawberry"]
```

### `group_by_field(field)`

Groups a list of hashmaps by a string field in a single O(n) pass. Returns a hashmap whose keys are the distinct values of `field` and whose values are lists of the matching rows. Insertion order of keys is preserved. Rows that are not hashmaps or that are missing `field` are silently skipped.

```kiwi
rows = [
  { "region": "West", "sales": 100 },
  { "region": "East", "sales": 200 },
  { "region": "West", "sales": 150 },
]

grouped = rows.group_by_field("region")
println grouped["West"].size()  # prints: 2
println grouped["East"].size()  # prints: 1
```

### `first(default = null)`

Returns the first value in a list. Returns `null` (or a provided default) if the list is empty.

```kiwi
println([1, 2, 3].first())     # prints: 1
println([].first("none"))      # prints: none
```

### `flatten()`

Flatten nested lists into a single list.

```kiwi
println([[1, 2, 3], [[4, 5], 6], [7]].flatten())
# prints: [1, 2, 3, 4, 5, 6, 7]
```

### `index(value)`

Returns the index of an item in a list. Returns -1 if not found.

```kiwi
println([1, 2, 3, 4, 5].index(1))  # prints: 0
println([1, 2, 3, 4, 5].index(6))  # prints: -1
```

### `insert(index, value)`

Insert a value at a specified index.

```kiwi
println([1, 2, 3].insert(2, "a")) # prints: [1, 2, "a", 3]
```

### `join(str)`

Joins a list into a string with an optional separator.

```kiwi
println(["Hello", "World!"].join(" ")) # prints: Hello World!
println([1, 2, 3].join(", "))          # prints: 1, 2, 3
```

### `last(default = null)`

Returns the last value in a list. Returns `null` (or a provided default) if the list is empty.

```kiwi
println([1, 2, 3].last())      # prints: 3
println([].last("none"))       # prints: none
```

### `lastindex(value)`

Returns the last index of an item in a list. Returns -1 if not found.

```kiwi
println([1, 0, 0, 1, 0, 1, 1].lastindex(1))  # prints: 6
println([1, 2, 3, 4, 5].lastindex(6))         # prints: -1
```

### `map(lambda)`

Transform each element in a list, returning a new list.

```kiwi
list = ["kiwi", "mango", "banana"]
println list.map(do (item) => item.uppercase())
# prints: ["KIWI", "MANGO", "BANANA"]
```

### `max()`

Get the highest value in a list.

```kiwi
println [1, 2, 3].max()      # prints: 3
println ["a", "z", "m"].max() # prints: z
```

### `min()`

Get the lowest value in a list.

```kiwi
println [1, 2, 3].min()       # prints: 1
println ["a", "z", "m"].min() # prints: a
```

### `none(lambda)`

Returns true if **no** elements in the list satisfy the predicate. The inverse of `all`.

```kiwi
list = [1, 3, 5, 7]
println list.none(do (n) => n % 2 == 0)  # prints: true  (no even numbers)

list.push(4)
println list.none(do (n) => n % 2 == 0)  # prints: false (4 is even)
```

### `pop()`

Returns and removes a value from the end of a list.

```kiwi
list = [1, 2, 3]
println list.pop() # prints: 3
println list       # prints: [1, 2]
```

### `push(value)`

Pushes a value onto the end of a list.

```kiwi
list = [1, 2, 3]
list.push(4)
println list  # prints: [1, 2, 3, 4]
```

### `reduce(accumulator, lambda)`

Aggregate the items in a list into a single value.

```kiwi
numbers = [1, 2, 3, 4, 5]
sum = numbers.reduce(0, do (acc, n) => acc + n)
println sum  # prints: 15
```

```kiwi
numbers = [1, 2, 3, 4, 5]
result = numbers.reduce({}, do (acc, n)
  acc["key${n}"] = n
  acc
end)
println result  # prints: {"key1": 1, "key2": 2, "key3": 3, "key4": 4, "key5": 5}
```

### `remove(value)`

Remove the first occurrence of a specific value in a list.

```kiwi
println([1, 2, 3].remove(2))           # prints: [1, 3]
println(["a", "b", 3, 4].remove("b"))  # prints: ["a", 3, 4]
```

### `remove_at(index)`

Remove a value from a list at a specified index.

```kiwi
println(["a", "b", 3, 4].remove_at(0)) # prints: ["b", 3, 4]
```

### `reverse()`

Reverse a list or a string.

```kiwi
println([1, 2, 3].reverse())   # prints: [3, 2, 1]
println("kiwi".reverse())      # prints: iwik
```

### `rotate(n)`

Rotate the values of the list by a specified number of positions.

If `n` is positive, values rotate right. If `n` is negative, values rotate left.

```kiwi
println "abcd".chars().rotate(1)  # prints: ["d", "a", "b", "c"]
println "abcd".chars().rotate(0)  # prints: ["a", "b", "c", "d"]
println "abcd".chars().rotate(-1) # prints: ["b", "c", "d", "a"]
```

### `shift()`

Removes and returns the first value of a list.

```kiwi
list = [1, 2, 3]
println(list.shift()) # prints: 1
println(list)         # prints: [2, 3]
```

### `size()`

Returns the size of a list, string, hashmap, or byte array as an integer.

```kiwi
println("four".size())            # prints: 4
println([1, 2, 3, true].size())   # prints: 4
println({a: 1, b: 2}.size())      # prints: 2
```

### `skip(n)`

Returns a new list with the first `n` elements removed.

```kiwi
println([1, 2, 3, 4, 5].skip(2))  # prints: [3, 4, 5]
println([1, 2, 3].skip(10))       # prints: []
```

### `slice(start, end)`

Get a subset of the list, specifying start (inclusive) and end (exclusive) indices.

```kiwi
println([1, 2, 3].slice(1, 2)) # prints: [2]
println([1, 2, 3].slice(0, 3)) # prints: [1, 2, 3]
println([1, 2, 3].slice(0, 2)) # prints: [1, 2]
```

### `sort()`

Sort a list in ascending order.

```kiwi
list = ["kiwi", "mango", "guava"]
println(list.sort()) # prints: ["guava", "kiwi", "mango"]

println([3, 1, 2].sort()) # prints: [1, 2, 3]
```

### `sort(lambda)`

Sort a list using a custom comparator. The lambda receives two elements and should return `true` if the first argument should come before the second.

```kiwi
# Sort descending
nums = [3, 1, 4, 1, 5, 9, 2, 6]
println nums.sort(do (a, b) => a > b)  # prints: [9, 6, 5, 4, 3, 2, 1, 1]

# Sort strings by length
words = ["banana", "fig", "kiwi", "mango"]
println words.sort(do (a, b) => a.size() < b.size())
# prints: ["fig", "kiwi", "mango", "banana"]
```

### `sum()`

Sum the numeric values in a list. Returns an integer if all values are integers, otherwise a float.

```kiwi
println([1, 2, 3].sum())         # prints: 6
println([1.5, 2.5, 3.0].sum())   # prints: 7.0
```

### `swap(index1, index2)`

Swaps two values in a list by index.

```kiwi
list = [1, 2, 3]
list.swap(0, 1)
println(list) # prints: [2, 1, 3]
```

### `take(n)`

Returns a new list containing only the first `n` elements.

```kiwi
println([1, 2, 3, 4, 5].take(3))  # prints: [1, 2, 3]
println([1, 2].take(10))          # prints: [1, 2]
```

### `to_bytes()`

Converts a string or list value to a byte array.

```kiwi
println("kiwi".to_bytes())         # byte array of UTF-8 encoded string
println("kiwi".chars().to_bytes()) # same result via chars
```

### `to_hex()`

Converts a byte array or list of integer byte values to a lowercase hexadecimal string.

```kiwi
println([97, 115, 116, 114, 97, 108].to_hex()) # prints: 61737472616c
println("kiwi".chars().to_bytes().to_hex())
```

### `unique()`

Remove duplicate values from the list.

```kiwi
println("aaaabbcccc".chars().unique()) # prints: ["a", "b", "c"]
println([1, 2, 2, 3, 3, 3].unique())  # prints: [1, 2, 3]
```

### `unshift(value)`

Inserts a value at the beginning of a list.

```kiwi
list = [1, 2, 3]
list.unshift(0)
println list  # prints: [0, 1, 2, 3]
```

### `zip(list)`

Combine values from two lists into pairs.

```kiwi
println([1, 2].zip([3, 4])) # prints: [[1, 3], [2, 4]]
println([1, 2, 3].zip(["a", "b", "c"])) # prints: [[1, "a"], [2, "b"], [3, "c"]]
```

## Conversion and Type Checking

### `between(a, b)`

See [Range Checking — `between`](#betweena-b).

### `clone()`

Returns a deep copy of the value.

```kiwi
list = [1, 2, 3, true, false]
list2 = list.clone()
list2[0] = "hello"

println(list)  # prints: [1, 2, 3, true, false]
println(list2) # prints: ["hello", 2, 3, true, false]
```

### `empty(default = null)`

Returns true if the value is a "default" (zero-like) value: empty string, empty list, empty hashmap, `0`, `0.0`, `false`, or `null`.

If a `default` argument is provided, returns that value when empty instead of `true`.

```kiwi
println((0).empty())    # prints: true
println("".empty())     # prints: true
println([].empty())     # prints: true
println({}.empty())     # prints: true
println(false.empty())  # prints: true
println("hi".empty())   # prints: false

# With a default value:
name = ""
println name.empty("anonymous")  # prints: anonymous

score = 100
println score.empty(0)            # prints: 100  (not empty, returns value itself)
```

### `is_a(type_name)`

Used for type-checking. Accepts a type name string or a struct reference. For struct instances, also checks the full inheritance hierarchy.

```kiwi
println("foobar".is_a(string))     # prints: true
println(42.is_a(integer))          # prints: true
println([].is_a(list))             # prints: true

struct Animal end
struct Dog < Animal end

d = Dog.new()
println d.is_a(Dog)     # prints: true
println d.is_a(Animal)  # prints: true  (checks hierarchy)
```

### `pretty()`

Returns a pretty-printed (indented) serialization of the value.

```kiwi
hashmap = {
  "key1": true,
  "key2": 1,
  "key3": ["a", "b", "c"]
}

println(hashmap.pretty())
/# prints:
{
  "key1": true,
  "key2": 1,
  "key3": [
    "a",
    "b",
    "c"
  ]
}
#/
```

### `to_date(format = null)`

Converts a string or integer to a date value.

- **String with no format**: parsed with `DateTime.TryParse` (ISO 8601 and common locale formats).
- **String with format**: parsed using the exact format string (e.g. `"dd/MM/yyyy"`).
- **Integer**: treated as a Unix timestamp in **milliseconds**.

Returns a default (zero) date if parsing fails.

```kiwi
d1 = "2024-06-15".to_date()
println d1.year()   # prints: 2024
println d1.month()  # prints: 6
println d1.day()    # prints: 15

# Custom format
d2 = "15/06/2024".to_date("dd/MM/yyyy")
println d2.year()   # prints: 2024

# From Unix timestamp (ms)
d3 = 1718409600000.to_date()
println d3.year()   # prints: 2024
```

### `to_float()`

Converts a numeric or string value to a float.

```kiwi
pi = "3.14159".to_float()
tau = pi * 2
println(tau) # 6.28318
```

### `to_integer()`

Converts a numeric or string value to an integer (truncates floats).

```kiwi
n = "100".to_integer()
n += 0.5
println(n)              # 100.5

println(n.to_integer()) # 100
```

### `to_list()`

Converts a string to a list of characters, or a byte array to a list of integer byte values.

```kiwi
println("abc".to_list())           # prints: ["a", "b", "c"]
println("kiwi".to_bytes().to_list()) # prints: [107, 105, 119, 105]
```

### `to_string(format = null)`

Converts a value to a string.

```kiwi
n = 100
s = [n, n.to_string()]
println(s)  # prints: [100, "100"]
```

#### Formatting Options

For integer and float values, an optional format specifier can be passed.

| Format | Name | Description | Example |
|--------|------|-------------|---------|
| `"B"` / `"b"` | Binary | 16-digit binary, zero-padded | `(31337).to_string("b")` -> `0111101001101001` |
| `"Fn"` / `"fn"` | Fixed-Point | `n` decimal places | `(100).to_string("f2")` -> `100.00` |
| `"O"` / `"o"` | Octal | Octal representation | `(64).to_string("o")` -> `100` |
| `"X"` | Hexadecimal (upper) | Uppercase hex | `(43).to_string("X")` -> `2B` |
| `"x"` | Hexadecimal (lower) | Lowercase hex | `(43).to_string("x")` -> `2b` |

### `truthy()`

Returns the truthiness of a value.

```kiwi
println null.truthy()   # prints: false  — null is never truthy
println (0).truthy()    # prints: false  — 0 is the only non-truthy integer
println (1).truthy()    # prints: true
println "".truthy()     # prints: false
println "0".truthy()    # prints: true   — non-empty strings are truthy
println [].truthy()     # prints: false
println [0].truthy()    # prints: true   — non-empty lists are truthy
println {}.truthy()     # prints: false  — empty hashmaps are not truthy
println true.truthy()   # prints: true
```

### `type()`

Returns the type of the value as a string.

Valid types: `integer`, `float`, `boolean`, `string`, `list`, `hashmap`, `lambda`, `date`, `bytes`, `null`. For struct instances, returns the struct name.

```kiwi
struct MyStruct end

instance = MyStruct.new()
println(instance.type())                  # prints: MyStruct
println("Kiwis are delicious!".type())    # prints: string
println(42.type())                        # prints: integer
println(3.14.type())                      # prints: float
```

### `deserialize(str)`

Deserializes a JSON-like string into a Kiwi value.

```kiwi
string = "[1, 2, 3]"
list = deserialize(string)
list.push(4)
println(list)  # prints: [1, 2, 3, 4]
```

### `serialize(value)`

Serializes a Kiwi value into its string representation.

```kiwi
data = { name: "kiwi", version: 1 }
println serialize(data)  # prints: {"name": "kiwi", "version": 1}
```

## Kiwi Global Builtins

These builtins are called as standalone functions (not with dot-notation) and provide runtime introspection.

### `typeof(value)`

Returns the type of `value` as a string. Unlike `.type()`, `typeof` can be called on any expression and works well in situations where method chaining is awkward.

Valid types: `integer`, `float`, `boolean`, `string`, `list`, `hashmap`, `lambda`, `date`, `bytes`, `null`. For struct instances, returns the struct name.

```kiwi
println typeof(42)          # prints: integer
println typeof(3.14)        # prints: float
println typeof("hello")     # prints: string
println typeof([1, 2, 3])   # prints: list
println typeof(null)        # prints: null

struct Point end
p = Point.new()
println typeof(p)           # prints: Point
```

### `__execpath__()`

Returns the absolute path of the Kiwi executable. Returns an empty string if the path cannot be determined.

```kiwi
println __execpath__()  # e.g. /usr/local/bin/kiwi
```

### `__entrypath__()`

Returns the path of the entry-point script (the `.kiwi` file that was passed to the interpreter). Useful for locating files relative to the running script.

```kiwi
println __entrypath__()  # e.g. /home/user/scripts/main.kiwi
```

### `__tokenize__(code)`

Lexes the string `code` using the Kiwi lexer and returns a hashmap. On success the hashmap contains a `"tokens"` key whose value is a list of token descriptors. On failure it contains an `"error"` key with the error message.

Each token descriptor is a hashmap with the following keys:

| Key | Type | Description |
|-----|------|-------------|
| `"token"` | integer | 1-based sequential token number |
| `"text"` | string | Raw token text |
| `"type"` | string | Token type (e.g. `"Keyword"`, `"Identifier"`, `"Literal"`) |
| `"span"` | hashmap | `{"line": n, "pos": n}` source location |

```kiwi
result = __tokenize__("x = 1 + 2")
for tok in result["tokens"]
  println "${tok["token"]}: '${tok["text"]}' (${tok["type"]}) @ ${tok["span"]["line"]}:${tok["span"]["pos"]}"
end
```

### `__memusage__()`

Returns a hashmap of memory diagnostics for the current runtime process.

| Key | Description |
|-----|-------------|
| `"working_set"` | OS working-set memory in bytes (resident pages) |
| `"gc_heap"` | Current .NET GC heap size in bytes |
| `"gc_total_allocated"` | Total bytes ever allocated on the GC heap |
| `"gen0_collections"` | Number of Gen-0 GC collections since startup |
| `"gen1_collections"` | Number of Gen-1 GC collections since startup |
| `"gen2_collections"` | Number of Gen-2 GC collections since startup |

```kiwi
mem = __memusage__()
mb = do (b) => (b / 1048576.0).to_string("f1") + " MB"

println "working set : ${mb(mem["working_set"])}"
println "gc heap     : ${mb(mem["gc_heap"])}"
println "gen0 / gen1 / gen2 collections: ${mem["gen0_collections"]} / ${mem["gen1_collections"]} / ${mem["gen2_collections"]}"
```
