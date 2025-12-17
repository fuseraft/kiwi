# Conditionals

In Kiwi, there are a few constructs for conditional logic.

## Table of Contents
1. [Conditions](#conditions)
2. [Non-Boolean Expressions as Conditions](#non-boolean-expressions-as-conditions)

## Conditions

A condition is a boolean expression that is passed to a [control structure](control_structures.md) to execute code when the condition is satisfied.

## Non-Boolean Expressions as Conditions

If a condition is a non-boolean expression, it will be evaluated as a boolean if it is *truthy* or *falsey*.

The following table will illustrate this.

| Type | Truthy Value | Falsey Value | Explanation |
| :--- | :--- | :--- | :--- |
| `integer` | `1` | `0` | A non-zero value is considered truthy. |
| `float` | `1.0` | `0.0` | A non-zero value is considered truthy. |
| `boolean` | `true` | `false` | These are the boolean values. |
| `string` | `"hello"` | `""` | A non-empty string is considered truthy. |
| `list` | `[1, 2, 3]` | `[]` | A non-empty list is considered truthy. |
| `hashmap` | `{"a": 1}` | `{}` | A non-empty hashmap is considered truthy. |

For more documentation on types, please see [Types](types.md).