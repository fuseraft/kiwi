# `time`

The `time` module contains functionality for working with time and date information.

## Importing the Module
To use the `time` module, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/time"
```

## Module Functions

### `hour()`
- **Summary**: Retrieves the current hour of the day (0 to 23).
- **Returns**: Integer representing the hour.

### `minute()`
- **Summary**: Retrieves the current minute of the hour (0 to 59).
- **Returns**: Integer representing the minute.

### `second()`
- **Summary**: Retrieves the current second of the minute (0 to 60).
- **Returns**: Integer representing the second.

### `ampm()`
- **Summary**: Retrieves the time of day designation for 12-hour clocks as "AM" or "PM".
- **Returns**: String either "AM" or "PM".

### `current_timestamp()`
- **Summary**: Retrieves the current timestamp formatted as "YYYY/mm/dd hh:mm:ss tt".
- **Returns**: String representing the formatted current timestamp.

### `day()`
- **Summary**: Retrieves the day of the month (1 to 31).
- **Returns**: Integer representing the day of the month.

### `month()`
- **Summary**: Retrieves the current month of the year (1 to 12).
- **Returns**: Integer representing the month.

### `year()`
- **Summary**: Retrieves the current year.
- **Returns**: Integer representing the year.

### `weekday()`
- **Summary**: Retrieves the day of the week (1 to 7, where 1 is Sunday and 7 is Saturday).
- **Returns**: Integer representing the day of the week.

### `yearday()`
- **Summary**: Retrieves the day of the year (0 to 365). December 31st is 364 on leap years.
- **Returns**: Integer representing the day of the year.

### `isdst()`
- **Summary**: Checks whether it is currently Daylight Savings Time.
- **Returns**: Boolean indicating DST status.

### `epochms()`
- **Summary**: Retrieves the number of milliseconds since January 1st, 1970.
- **Returns**: Double representing milliseconds since the epoch.

### `ticks()`
- **Summary**: Retrieves the number of nanoseconds since January 1st, 1970.
- **Returns**: Double representing nanoseconds since the epoch.

### `ticksms(@_ticks)`
- **Summary**: Converts ticks to milliseconds.
- **Returns**: Double representing milliseconds.

### `delay(@_ms)`
- **Parameters**:
  - `@_ms`: The duration to sleep in milliseconds.
- **Summary**: Pauses execution for a specified duration in milliseconds.
- **Returns**: Double representing the duration paused.
