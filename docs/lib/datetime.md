# Dates and Times

## `DateTime`

The `DateTime` struct is used to represent date and time data.

### Example

```kiwi
# print todays date.
println(DateTime.format(DateTime.now().add_years(1), "%Y-%m-%d"))

d = DateTime.new(2024, 1, 1).add_days(-1).add_hours(-1).add_minutes(-1).add_seconds(-1)
println(DateTime.format(d, "%Y-%m-%dT%H:%M:%S")) # prints: 2023-12-30T22:58:59
```

### Methods
| Type | Returns | Method | Description |
| :--- | :--- | :--- | :--- |
| Constructor | `DateTime` | `new(year, month, day, hour, minute, second)` | Initialize a new `DateTime`. <br>`year` defaults to `1970`. <br>`month` and `day` default to `1`. <br>`hour`, `minute`, and `second` default to `0`.|
| Static | `String` | `format(_datetime, _format)` | Format the value with a given format string. <br>`_format` defaults to `"%Y-%m-%d %H:%M:%S"`. |
| Static | `DateTime` | `now()` | Returns the current date and time as a `DateTime` object. |
| Instance | `String` | `to_string()` | Returns a string representation of the `DateTime` object. |
| instance | `List` | `to_number()` | Returns the date and time parts as a list of integers. |
| instance | `Boolean` | `equal(another_date)` | Returns `true` if the values are equal.<br>`another_date` must be a `DateTime`. |
| instance | `Boolean` | `after(another_date)` | Returns `true` if the value is greater than another `DateTime`.<br>`another_date` must be a `DateTime`. |
| instance | `Boolean` | `before(another_date)` | Returns `true` if the value is less than another `DateTime`.<br>`another_date` must be a `DateTime`. |
| instance | `DateTime` | `add_days(n)` | Adds `n` number of days. |
| instance | `DateTime` | `add_months(n)` | Adds `n` number of months. |
| instance | `DateTime` | `add_years(n)` | Adds `n` number of years. |
| instance | `DateTime` | `add_hours(n)` | Adds `n` number of hours. |
| instance | `DateTime` | `add_minutes(n)` | Adds `n` number of minutes. |
| instance | `DateTime` | `add_seconds(n)` | Adds `n` number of seconds. |
| instance | `Integer` | `get_year()` | Gets the year. |
| instance | `Integer` | `get_month()` | Gets the month. |
| instance | `Integer` | `get_day()` | Gets the day. |
| instance | `Integer` | `get_hour()` | Gets the hour. |
| instance | `Integer` | `get_minute()` | Gets the minute. |
| instance | `Integer` | `get_second()` | Gets the second. |

## `TimeSpan`

The `TimeSpan` struct is used to represent a duration of time between two `DateTime` objects or a period defined by the user.

### Example

```kiwi
start = DateTime.new(2023, 1, 1)
end = DateTime.new(2024, 1, 1)
duration = end.diff(start)
println(duration.to_string()) # prints: TimeSpan: 1 years, 0 months, 0 days, 0 hours, 0 minutes, 0 seconds
```

### Methods
| Type | Returns | Method | Description |
| :--- | :--- | :--- | :--- |
| Constructor | `TimeSpan` | `new(year, month, day, hour, minute, second)` | Initialize a new `TimeSpan` with the given values. <br> All parameters default to `0`. |
| Instance | `Integer` | `get_year()` | Returns the number of years in the `TimeSpan`. |
| Instance | `Integer` | `get_month()` | Returns the number of months in the `TimeSpan`. |
| Instance | `Integer` | `get_day()` | Returns the number of days in the `TimeSpan`. |
| Instance | `Integer` | `get_hour()` | Returns the number of hours in the `TimeSpan`. |
| Instance | `Integer` | `get_minute()` | Returns the number of minutes in the `TimeSpan`. |
| Instance | `Integer` | `get_second()` | Returns the number of seconds in the `TimeSpan`. |
| Instance | `String` | `to_string()` | Returns a string representation of the `TimeSpan` object. |
