# Variables

Variable value types are dynamic and change depending on assignment.

Variable names must begin with either a letter or an underscore.

To see all of the types Kiwi variables can store, see [types](types.md).

```ruby
name = "Kiwi"
powerlevel = 9001
threshold = 9000

println "I hope you like ${name}!"

if powerlevel > threshold
  println "It's over ${threshold}!"
end
```