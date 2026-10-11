using System.Collections;
using System.Diagnostics.CodeAnalysis;

namespace kiwi.Typing;

/// <summary>
/// Storage behind Kiwi hashmaps and object/static variables. Tasks run on separate
/// threads and can share these, so every operation takes the dictionary's lock.
/// Enumeration, <see cref="Keys"/> and <see cref="Values"/> return snapshots, which keeps
/// callers (and any user lambdas they run) from executing while the lock is held.
/// Iteration order is insertion order, as with <see cref="Dictionary{TKey, TValue}"/>.
/// </summary>
public sealed class SyncDictionary<TKey, TValue> : IEnumerable<KeyValuePair<TKey, TValue>>
    where TKey : notnull
{
    private readonly Lock _sync = new();
    private readonly Dictionary<TKey, TValue> _items;

    public SyncDictionary() => _items = [];

    public SyncDictionary(IEqualityComparer<TKey>? comparer) => _items = new Dictionary<TKey, TValue>(comparer);

    public SyncDictionary(IEnumerable<KeyValuePair<TKey, TValue>> items) => _items = new Dictionary<TKey, TValue>(items);

    private SyncDictionary(Dictionary<TKey, TValue> items, bool _) => _items = items;

    /// <summary>
    /// Wraps <paramref name="items"/> without copying. The caller must not touch
    /// <paramref name="items"/> afterwards; all access has to go through the wrapper.
    /// </summary>
    public static SyncDictionary<TKey, TValue> Adopt(Dictionary<TKey, TValue> items) => new(items, true);

    public int Count { get { lock (_sync) return _items.Count; } }

    public TValue this[TKey key]
    {
        get { lock (_sync) return _items[key]; }
        set { lock (_sync) _items[key] = value; }
    }

    public List<TKey> Keys { get { lock (_sync) return [.. _items.Keys]; } }

    public List<TValue> Values { get { lock (_sync) return [.. _items.Values]; } }

    public bool TryGetValue(TKey key, [MaybeNullWhen(false)] out TValue value)
    {
        lock (_sync) return _items.TryGetValue(key, out value);
    }

    public TValue? GetValueOrDefault(TKey key)
    {
        lock (_sync) return _items.GetValueOrDefault(key);
    }

    public TValue GetValueOrDefault(TKey key, TValue defaultValue)
    {
        lock (_sync) return _items.GetValueOrDefault(key, defaultValue);
    }

    public bool ContainsKey(TKey key) { lock (_sync) return _items.ContainsKey(key); }

    public bool ContainsValue(TValue value) { lock (_sync) return _items.ContainsValue(value); }

    public void Add(TKey key, TValue value) { lock (_sync) _items.Add(key, value); }

    public bool TryAdd(TKey key, TValue value) { lock (_sync) return _items.TryAdd(key, value); }

    public bool Remove(TKey key) { lock (_sync) return _items.Remove(key); }

    public bool Remove(TKey key, [MaybeNullWhen(false)] out TValue value)
    {
        lock (_sync) return _items.Remove(key, out value);
    }

    public void Clear() { lock (_sync) _items.Clear(); }

    /// <summary>
    /// Runs a multi-step update on the underlying dictionary while holding the lock, so other
    /// tasks see it as one change. <paramref name="action"/> must not call back into Kiwi code.
    /// </summary>
    public void Mutate(Action<Dictionary<TKey, TValue>> action) { lock (_sync) action(_items); }

    /// <inheritdoc cref="Mutate(Action{Dictionary{TKey, TValue}})"/>
    public T Mutate<T>(Func<Dictionary<TKey, TValue>, T> func) { lock (_sync) return func(_items); }

    /// <summary>A new, unsynchronized copy of the contents.</summary>
    public Dictionary<TKey, TValue> ToDictionary()
    {
        lock (_sync) return new Dictionary<TKey, TValue>(_items, _items.Comparer);
    }

    public KeyValuePair<TKey, TValue>[] ToArray() { lock (_sync) return [.. _items]; }

    public IEnumerator<KeyValuePair<TKey, TValue>> GetEnumerator() =>
        ((IEnumerable<KeyValuePair<TKey, TValue>>)ToArray()).GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
}
