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

:::caution
`to_hash()` currently fails with an `InvalidOperationError`, because `@` isn't available inside the lambda it uses. Until that is fixed, build the hashmap from `.keys()` and `.values()`:

```kiwi
struct Config
  fn new(host, port)
    @host = host
    @port = port
  end
end

cfg = Config.new("localhost", 8080)
hash = {}
for name, i in cfg.keys() do
  hash[name] = cfg.values()[i]
end
println hash   # {"host": "localhost", "port": 8080}
```
:::

## `Hashable` struct

When inherited, structs receive a to_hash() function that returns their instance variables as a hashmap.

### `.to_hash`

```kiwi
to_hash(): hashmap
```

Returns instance variables as a hashmap.

**Returns** `hashmap` — Returns a hashmap containing instance variables as key-value pairs.
