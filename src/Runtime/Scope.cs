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
    public Scope? Parent { get; } = parent;

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
            if (cur._locals.ContainsKey(name))
            {
                cur._locals[name] = value;
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