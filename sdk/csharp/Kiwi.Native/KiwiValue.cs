namespace Kiwi.Native;

/// <summary>
/// A handle to a Kiwi value. It is valid only until the native call that received or created
/// it returns, so never store one; read it with a <see cref="KiwiContext"/> instead.
/// </summary>
public readonly struct KiwiValue(ulong handle) : IEquatable<KiwiValue>
{
    /// <summary>The raw handle. 0 means no value, which Kiwi reads as <c>null</c>.</summary>
    public ulong Handle { get; } = handle;

    /// <summary>No value. Returned to Kiwi, it is <c>null</c>.</summary>
    public static KiwiValue Empty => default;

    public bool IsEmpty => Handle == 0;

    public bool Equals(KiwiValue other) => Handle == other.Handle;
    public override bool Equals(object? obj) => obj is KiwiValue other && Equals(other);
    public override int GetHashCode() => Handle.GetHashCode();
    public override string ToString() => $"KiwiValue({Handle})";

    public static bool operator ==(KiwiValue left, KiwiValue right) => left.Equals(right);
    public static bool operator !=(KiwiValue left, KiwiValue right) => !left.Equals(right);
}
