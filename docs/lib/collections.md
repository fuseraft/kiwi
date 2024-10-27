# `collections`

The `collections` package provides data structures for handling heaps and sets. This documentation covers the available structures and their functionalities.

This is WIP and we will continue to add new definitions to this package over time.

## Overview

This package contains two primary structures:
1. **Heap**: An implementation for managing heaps, supporting both min-heaps and max-heaps.
2. **Set**: An implementation for managing unique collections of data with various set operations.

## Heap Structure

The `Heap` structure allows for the creation of either a min-heap or a max-heap. The heap supports basic operations such as inserting elements, extracting the root, and peeking at the root.

### Constructor

```kiwi
fn new(type = "min")
```

- **Parameters**:
  - `type`: A string specifying the type of heap. It can be either `"min"` or `"max"`. The default is `"min"`. If an invalid type is provided, it defaults to `"min"`.

### Public Methods

#### `fn insert(value)`
Inserts a new value into the heap and maintains the heap property.

- **Parameters**:
  - `value`: The value to be inserted into the heap.

#### `fn extract_root()`
Removes and returns the root element of the heap. Throws an error if the heap is empty.

- **Returns**: 
  - The root element of the heap.

#### `fn peek()`
Returns the root element without removing it. Throws an error if the heap is empty.

#### `fn size()`
Returns the number of elements in the heap.

#### `fn is_empty()`
Returns whether the heap is empty.

### Private Methods

#### `fn heapify_up(index)`
Ensures the heap property is maintained after inserting a new element.

- **Parameters**:
  - `index`: The index of the newly inserted element.

#### `fn heapify_down(index)`
Ensures the heap property is maintained after removing the root element.

- **Parameters**:
  - `index`: The index of the element being moved down the heap.

#### `fn swap(i, j)`
Swaps the elements at positions `i` and `j` in the heap.

- **Parameters**:
  - `i`: The index of the first element.
  - `j`: The index of the second element.

---

## Set Structure

The `Set` structure provides a way to manage collections of unique elements. It supports various set operations such as union, intersection, and difference.

### Constructor

```kiwi
fn new(data = [])
```

- **Parameters**:
  - `data`: A list of initial values for the set. The values are automatically made unique.

### Public Methods

#### `fn add(item)`
Adds a new item to the set, ensuring that the set remains unique.

- **Parameters**:
  - `item`: The item to be added to the set.

#### `fn remove(item)`
Removes the specified item from the set.

- **Parameters**:
  - `item`: The item to be removed.

#### `fn contains(item)`
Checks whether the set contains the specified item.

- **Returns**: 
  - `true` if the set contains the item, `false` otherwise.

#### `fn size()`
Returns the number of elements in the set.

#### `fn clear()`
Removes all elements from the set.

#### `fn difference(data)`
Returns a new set containing elements that are in the current set but not in the provided data.

- **Parameters**:
  - `data`: A list or another set to compare with.

#### `fn disjoint(data)`
Returns `true` if the set has no elements in common with the provided data.

#### `fn intersect(data)`
Checks if the set shares any elements with the provided data.

- **Returns**: 
  - `true` if the sets intersect, `false` otherwise.

#### `fn merge(data)`
Merges the provided data into the current set.

- **Parameters**:
  - `data`: A list or set to merge with the current set.

#### `fn union(data)`
Returns a new set that is the union of the current set and the provided data.

- **Parameters**:
  - `data`: A list or set to combine with the current set.

#### `fn subset(data)`
Checks whether the current set is a subset of the provided data.

- **Returns**: 
  - `true` if the current set is a subset of the provided data, `false` otherwise.

#### `fn superset(data)`
Checks whether the current set is a superset of the provided data.

- **Returns**: 
  - `true` if the current set is a superset, `false` otherwise.

#### `fn to_list()`
Returns a list of the set's elements.

#### `fn to_string()`
Returns a string representation of the set.

#### `fn type()`
Returns the string `"Set"` to identify the type of the structure.

### Private Methods

#### `fn get_input_data(data)`
Extracts the input data from the provided data. It handles both sets and lists, throwing an error if the data is not valid.

- **Parameters**:
  - `data`: The input data, either a set or a list.
