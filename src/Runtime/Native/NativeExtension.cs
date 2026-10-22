namespace kiwi.Runtime.Native;

/// <summary>
/// A loaded native extension library and the package its functions live in.
/// </summary>
public sealed class NativeExtension(string path, string packageName, nint library)
{
    public string Path { get; } = path;
    public string PackageName { get; } = packageName;
    public nint Library { get; } = library;
    public List<string> Functions { get; } = [];
}
