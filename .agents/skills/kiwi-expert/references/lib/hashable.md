# hashable

> A base struct that converts instances to hashmaps.

The `hashable` package defines `Hashable`, a base struct whose `to_hash()` method returns an instance's variables as a hashmap. Inherit from it to make a struct easy to serialize or log.

```kiwi
struct Config < Hashable
  fn new(host, port)
    @host = host
    @port = port
  end
end

println Config.new("localhost", 8080).to_hash()   # {"host": "localhost", "port": 8080}
```

## `Hashable` struct

When inherited, structs receive a to_hash() function that returns their instance variables as a hashmap.

### `.to_hash`

```kiwi
to_hash(): hashmap
```

Returns instance variables as a hashmap.

**Returns** `hashmap` — Returns a hashmap containing instance variables as key-value pairs.
