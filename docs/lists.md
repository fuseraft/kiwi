# Lists

*Under construction in new interpreter.*

Lists in Kiwi look and behave a lot like lists in other languages.

### Defining a List

```ruby
@myList = ["Kiwi", "is", "fun!"]

@anotherList = [1, 2, 3, "four", "five", 6]
```

### Accessing List Elements

Bracket notation can be used to access elements by index, starting from 0.

```ruby
println "${@myList[0]}"  # Outputs: Kiwi
println @myList[2]       # Outputs: fun!
```

### Adding Elements to a List

```ruby
@myList = []
@anotherList = [1, 2];

@myList << "Hello, Kiwi!"
@myList << @anotherList

# @myList now contains ["Hello, Kiwi!", [1, 2]]
```

### Iterating a List

Use the `for` keyword to iterate a list.

```ruby
# Iterate the values in the list.
for @item in @myList do
  println "${@item}"
end

# Iterate the values in the list, with an index.
for @item, @index in @myList do
  println "Item ${@index}: ${@item}"
end
```

### Defining a List with a Range

You can define a list using a range.

```ruby
# This is equivalent to: @numbers = [1, 2, 3, 4, 5]
@numbers = [1..5] 
```

### An Example

```ruby
def sieveOfEratosthenes(@limit)
  @isPrime = []
  for @i in [0..@limit]
     @isPrime << true
  end

  @isPrime[0] = false  # 0 is not a prime
  @isPrime[1] = false  # 1 is not a prime
  
  @p = 2  # The first prime number
  
  while @p * @p <= @limit do
    # If @p is not changed, then it is a prime
    if @isPrime[@p]
      # Update all multiples of @p
      @multiple = @p * 2
      
      while @multiple <= @limit do
        @isPrime[@multiple] = false
        @multiple += @p
      end
    endif

    @p += 1
  end

  # Collecting all prime numbers
  @primes = []
  for @i in [0..@limit] do
    if @isPrime[@i]
      @primes << (@i)
    endif
  end

  return @primes
end

for @prime, @i in sieveOfEratosthenes(30) do
  println "${@i}:\t${@prime}"
end
```