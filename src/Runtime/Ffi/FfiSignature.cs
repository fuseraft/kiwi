namespace kiwi.Runtime.Ffi;

/// <summary>
/// A C function's name, parameter types and return type. Signatures are kept for the life of
/// the process and referred to from Kiwi by number, so a bad number is an error rather than a
/// crash.
/// </summary>
public sealed class FfiSignature(string name, FfiType[] parameters, FfiType returns)
{
    private static readonly List<FfiSignature> _all = [];
    private static readonly Dictionary<string, int> _ids = [];
    private static readonly object _lock = new();

    public string Name { get; } = name;
    public FfiType[] Parameters { get; } = parameters;
    public FfiType Returns { get; } = returns;

    /// <summary>The number of an equal signature, registering this one if it is new.</summary>
    public static int Register(FfiSignature signature)
    {
        var key = signature.Name + "(" + string.Join(",", signature.Parameters) + ")" + signature.Returns;
        lock (_lock)
        {
            if (_ids.TryGetValue(key, out var id))
                return id;
            _all.Add(signature);
            _ids[key] = _all.Count - 1;
            return _all.Count - 1;
        }
    }

    public static FfiSignature? Get(long id)
    {
        lock (_lock)
            return id >= 0 && id < _all.Count ? _all[(int)id] : null;
    }
}
