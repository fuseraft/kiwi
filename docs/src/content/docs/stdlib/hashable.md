---
title: hashable
description: A base struct that converts instances to hashmaps.
---

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

<!-- api: hashable -->
