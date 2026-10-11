using System.Collections;

namespace kiwi.Typing;

/// <summary>
/// Storage behind a Kiwi list. Tasks run on separate threads and can share lists, so
/// every operation takes the list's lock. Enumeration walks a snapshot, which keeps
/// callers (and any user lambdas they run) from executing while the lock is held.
/// </summary>
public sealed class SyncList : IList<Value>, IReadOnlyList<Value>
{
    private readonly Lock _sync = new();
    private readonly List<Value> _items;

    public SyncList() => _items = [];

    public SyncList(int capacity) => _items = new List<Value>(capacity);

    public SyncList(IEnumerable<Value> items) => _items = [.. items];

    private SyncList(List<Value> items, bool _) => _items = items;

    /// <summary>
    /// Wraps <paramref name="items"/> without copying. The caller must not touch
    /// <paramref name="items"/> afterwards; all access has to go through the wrapper.
    /// </summary>
    public static SyncList Adopt(List<Value> items) => new(items, true);

    public int Count { get { lock (_sync) return _items.Count; } }

    public bool IsReadOnly => false;

    public Value this[int index]
    {
        get { lock (_sync) return _items[index]; }
        set { lock (_sync) _items[index] = value; }
    }

    /// <summary>Reads <paramref name="index"/> if it is in range, in one locked step.</summary>
    public bool TryGet(int index, out Value value)
    {
        lock (_sync)
        {
            if ((uint)index < (uint)_items.Count)
            {
                value = _items[index];
                return true;
            }
        }
        value = Value.Default;
        return false;
    }

    public void Add(Value item) { lock (_sync) _items.Add(item); }

    public void AddRange(IEnumerable<Value> items)
    {
        // Snapshot first: `items` may be this list or another SyncList.
        var copy = items is SyncList other ? other.ToArray() : items.ToArray();
        lock (_sync) _items.AddRange(copy);
    }

    public void Insert(int index, Value item) { lock (_sync) _items.Insert(index, item); }

    public void InsertRange(int index, IEnumerable<Value> items)
    {
        var copy = items is SyncList other ? other.ToArray() : items.ToArray();
        lock (_sync) _items.InsertRange(index, copy);
    }

    public bool Remove(Value item) { lock (_sync) return _items.Remove(item); }

    public void RemoveAt(int index) { lock (_sync) _items.RemoveAt(index); }

    public void RemoveRange(int index, int count) { lock (_sync) _items.RemoveRange(index, count); }

    /// <summary>
    /// Removes matching items. <paramref name="match"/> runs under the lock, so it must
    /// not call back into Kiwi code.
    /// </summary>
    public int RemoveAll(Predicate<Value> match) { lock (_sync) return _items.RemoveAll(match); }

    public void Clear() { lock (_sync) _items.Clear(); }

    public bool Contains(Value item) { lock (_sync) return _items.Contains(item); }

    public int IndexOf(Value item) { lock (_sync) return _items.IndexOf(item); }

    public int IndexOf(Value item, int index) { lock (_sync) return _items.IndexOf(item, index); }

    public int LastIndexOf(Value item) { lock (_sync) return _items.LastIndexOf(item); }

    public List<Value> GetRange(int index, int count) { lock (_sync) return _items.GetRange(index, count); }

    public void Reverse() { lock (_sync) _items.Reverse(); }

    /// <summary>
    /// Sorts a snapshot outside the lock (the comparison may run Kiwi code), then
    /// replaces the contents.
    /// </summary>
    public void Sort(Comparison<Value> comparison)
    {
        var copy = ToList();
        copy.Sort(comparison);
        ReplaceWith(copy);
    }

    public void Sort(IComparer<Value> comparer)
    {
        var copy = ToList();
        copy.Sort(comparer);
        ReplaceWith(copy);
    }

    /// <summary>
    /// Runs a multi-step update on the underlying list while holding the lock, so other
    /// tasks see it as one change. <paramref name="action"/> must not call back into Kiwi code.
    /// </summary>
    public void Mutate(Action<List<Value>> action) { lock (_sync) action(_items); }

    /// <inheritdoc cref="Mutate(Action{List{Value}})"/>
    public T Mutate<T>(Func<List<Value>, T> func) { lock (_sync) return func(_items); }

    /// <summary>Replaces the contents in one locked step.</summary>
    public void ReplaceWith(IEnumerable<Value> items)
    {
        var copy = items is SyncList other ? other.ToArray() : items.ToArray();
        lock (_sync)
        {
            _items.Clear();
            _items.AddRange(copy);
        }
    }

    public Value[] ToArray() { lock (_sync) return _items.ToArray(); }

    /// <summary>A new, unsynchronized copy of the contents.</summary>
    public List<Value> ToList() { lock (_sync) return new List<Value>(_items); }

    public void CopyTo(Value[] array, int arrayIndex) { lock (_sync) _items.CopyTo(array, arrayIndex); }

    public IEnumerator<Value> GetEnumerator() => ((IEnumerable<Value>)ToArray()).GetEnumerator();

    IEnumerator IEnumerable.GetEnumerator() => GetEnumerator();
}
