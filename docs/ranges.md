# Ranges

Ranges in Kiwi can be used to define a range of integers.

###

```kiwi
println([1..5])
# [1, 2, 3, 4, 5]

# Build a list of numbers from 1 to 10 inclusive, then reverse it.
range = [1..10] 
range = range[::-1]

println(range) 
# [10, 9, 8, 7, 6, 5, 4, 3, 2, 1]

range = [20..18]
println(range)
# [20, 19, 18]
```