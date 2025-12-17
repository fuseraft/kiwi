# Dates

The `date` type in Kiwi is used to represent a date and a time.

There is no Date literal syntax in Kiwi, but the language provides methods to access and use the type.

# Table of Contents
- [Builtins](#builtins)
- [Get Current Date and Time](#get-current-date-and-time)
- [`string`s to `date`s](#strings-to-dates)
- [`date`s to `string`s](#dates-to-strings)

### Builtins

For documentation on `date` builtins, take a look at the [`date` builtins](builtins.md#date-builtins).

### Get Current Date and Time

Using the standard library call `time::now()` or special builtin `__time_now__()`.

```kiwi
dt = time::now()     # using the `time` package from the Kiwi Standard Library.
dt = __time_now__()  # using the special builtin `__time_now__()`
```

### `string`s to `date`s

Use the `to_date()` builtin to convert a string to a date.

```kiwi
dt = "2021-01-01".to_date()

println dt # prints: 1/1/2021 12:00:00 AM
```

### `date`s to `string`s

Use the `to_string()` builtin to convert a date to a string. Optionally, you can specify a format.

```kiwi
dt = "2021-01-02".to_date().to_string("MM/dd/yyyy")
println dt # prints: 01/02/2021
```