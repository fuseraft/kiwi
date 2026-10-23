using System.Runtime.InteropServices;

namespace Kiwi.Native;

/// <summary>
/// Keeps a .NET object alive between native calls and hands it to Kiwi as a pointer. Kiwi can't
/// free it, so give the extension a function that calls <see cref="Free"/>.
/// </summary>
/// <example>
/// <code>
/// [KiwiFunction] public static nint CounterNew() => KiwiHandle.Alloc(new Counter());
/// [KiwiFunction] public static long CounterBump(nint counter) => KiwiHandle.Get&lt;Counter&gt;(counter).Bump();
/// [KiwiFunction] public static void CounterFree(nint counter) => KiwiHandle.Free(counter);
/// </code>
/// </example>
public static class KiwiHandle
{
    public static nint Alloc(object value) => GCHandle.ToIntPtr(GCHandle.Alloc(value));

    /// <summary>
    /// The object behind a pointer from <see cref="Alloc"/>. Don't use a pointer after freeing
    /// it: its slot can be reused for another object.
    /// </summary>
    /// <exception cref="KiwiException">The pointer is null, freed, or holds another type.</exception>
    public static T Get<T>(nint handle) where T : class
    {
        if (handle == 0)
            throw new KiwiException("The pointer is null.");
        try
        {
            return GCHandle.FromIntPtr(handle).Target as T
                ?? throw new KiwiException($"The pointer doesn't hold a {typeof(T).Name}, or was freed.");
        }
        catch (InvalidOperationException)
        {
            throw new KiwiException("The pointer was freed.");
        }
    }

    public static void Free(nint handle)
    {
        if (handle == 0) return;
        GCHandle.FromIntPtr(handle).Free();
    }
}
