using System.Runtime.InteropServices;
using kiwi.Typing;

namespace kiwi.Runtime;

/// <summary>
/// Represents lexical scope.
/// </summary>
/// <param name="parent"></param>
public class Scope(Scope? parent = null)
{
    /// <summary>
    /// Local bindings of the current scope.
    /// </summary>
    private readonly Dictionary<string, Value> _locals = [];

    /// <summary>
    /// Gets the parent scope.
    /// </summary>
    public Scope? Parent { get; private set; } = parent;

    /// <summary>
    /// True when this scope was rented from the block-scope pool (and is therefore eligible
    /// to be returned to the pool when its block exits, provided nothing captured it).
    /// </summary>
    internal bool IsBlockScope { get; private set; }

    /// <summary>
    /// True when a lambda or nested function captured this scope (or a child block scope
    /// whose parent chain includes this scope). Captured scopes are never returned to the pool.
    /// </summary>
    private bool _captured;

    /// <summary>
    /// Resets a pooled scope for reuse as a block scope.
    /// </summary>
    internal void Reset(Scope? newParent)
    {
        Parent = newParent;
        IsBlockScope = true;
        _captured = false;
        _locals.Clear();
    }

    /// <summary>
    /// Marks this scope and all parent block scopes as captured so they are not returned
    /// to the pool. Called whenever a lambda or function definition closes over the current scope.
    /// </summary>
    internal void MarkCaptured()
    {
        var cur = this;
        while (cur != null && cur.IsBlockScope && !cur._captured)
        {
            cur._captured = true;
            cur = cur.Parent;
        }
    }

    /// <summary>
    /// True when it is safe to return this scope to the block-scope pool.
    /// </summary>
    internal bool CanPool => IsBlockScope && !_captured;

    /// <summary>
    /// Perform a lexical search to lookup a binding in the scope chain and return its value.
    /// </summary>
    /// <param name="name">The name of the binding.</param>
    /// <param name="value">The value of the binding (if found).</param>
    /// <returns>Returns <c>true<c> if the binding is found.</returns>
    public bool TryGet(string name, out Value value)
    {
        var cur = this;
        while (cur != null)
        {
            if (cur._locals.TryGetValue(name, out Value? val))
            {
                value = val;
                return true;
            }
            cur = cur.Parent;
        }
        value = Value.Default;
        return false;
    }

    public Value GetBinding(string name)
    {
        if (TryGet(name, out Value? val))
        {
            return val;
        }

        return Value.Default;
    }

    /// <summary>
    /// Updates the nearest existing binding or creates a new one.
    /// </summary>
    /// <param name="name">The name of the binding</param>
    /// <param name="value">The new value.</param>
    public void Assign(string name, Value value)
    {
        var cur = this;
        while (cur != null)
        {
            ref Value slot = ref CollectionsMarshal.GetValueRefOrNullRef(cur._locals, name);
            if (!System.Runtime.CompilerServices.Unsafe.IsNullRef(ref slot))
            {
                slot = value;
                return;
            }
            cur = cur.Parent;
        }

        // not found, create in the current scope
        _locals[name] = value;
    }

    /// <summary>
    /// Declares a binding. Always creates a new binding in the current scope.
    /// </summary>
    /// <param name="name">The name of the binding.</param>
    /// <param name="value">Its value.</param>
    public void Declare(string name, Value value)
    {
        _locals[name] = value ?? Value.Default;
    }

    /// <summary>
    /// Directly updates a binding in the current scope's locals, bypassing the chain walk.
    /// Only call when the binding is guaranteed to be in this scope (e.g., loop iterators).
    /// </summary>
    public void SetLocal(string name, Value value) => _locals[name] = value;

    public void Remove(string name)
    {
        _locals.Remove(name);
    }

    /// <summary>
    /// Gets all bindings.
    /// </summary>
    /// <returns>A list of bindings.</returns>
    public IEnumerable<KeyValuePair<string, Value>> GetAllBindings()
    {
        for (var current = this; current != null; current = current.Parent)
            foreach (var kv in current._locals)
                yield return kv;
    }
}
