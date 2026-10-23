namespace Kiwi.Native;

/// <summary>
/// An error to report to Kiwi. Any exception a <see cref="KiwiFunctionAttribute"/> method
/// throws becomes a <c>NativeError</c> with its message; this one is also what the SDK throws
/// for a value of the wrong type.
/// </summary>
public class KiwiException(string message) : Exception(message);
