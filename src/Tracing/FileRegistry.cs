namespace kiwi.Tracing;

public sealed class FileRegistry
{
    private static readonly Lazy<FileRegistry> _instance = new(() => new FileRegistry());

    public static FileRegistry Instance => _instance.Value;

    private int _nextId;
    private readonly Dictionary<int, string> _registry;

    private FileRegistry()
    {
        _nextId = 0;
        _registry = [];
    }

    public int RegisterFile(string filePath)
    {
        // Check if file is already registered
        int fileId = GetFileId(filePath);
        if (fileId > -1)
        {
            return fileId;
        }

        int id = _nextId++;
        _registry[id] = Path.GetFullPath(filePath);

        return id;
    }

    public int GetFileId(string filePath)
    {
        foreach (var pair in _registry)
        {
            if (pair.Value == filePath)
            {
                return pair.Key;
            }
        }
        
        return -1;
    }

    public string GetFilePath(int id)
    {
        if (_registry.TryGetValue(id, out var path))
        {
            return path;
        }

        return string.Empty;
    }

    public List<string> GetFileLines(int id)
    {
        var filePath = GetFilePath(id);
        return [.. File.ReadAllLines(filePath)];
    }
}