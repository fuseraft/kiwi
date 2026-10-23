namespace kiwi.Runtime.Ffi;

/// <summary>A problem with an <c>ffi</c> call or memory access, reported in Kiwi as an <c>FfiError</c>.</summary>
public sealed class FfiException(string message) : Exception(message);
