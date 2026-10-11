namespace kiwi.Tracing;

public sealed class FileRegistry
{
    private static readonly Lazy<FileRegistry> _instance = new(() => new FileRegistry());

    public static FileRegistry Instance => _instance.Value;

    private int _nextId;
    private readonly Dictionary<int, string> _registry;
    // Files are registered by every Lexer, including ones created on task threads.
    private readonly Lock _sync = new();

    private FileRegistry()
    {
        _nextId = 0;
        _registry = [];
    }

    public int RegisterFile(string filePath)
    {
        lock (_sync) return RegisterFileLocked(filePath);
    }

    private int RegisterFileLocked(string filePath)
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
        string fullPath;
        try
        {
            fullPath = Path.GetFullPath(filePath);
        }
        catch
        {
            fullPath = filePath;
        }

        lock (_sync)
        {
            foreach (var pair in _registry)
            {
                if (pair.Value == fullPath || pair.Value == filePath)
                {
                    return pair.Key;
                }
            }
        }

        return -1;
    }

    public string GetFilePath(int id)
    {
        lock (_sync)
        {
            if (_registry.TryGetValue(id, out var path))
            {
                return path;
            }
        }

        return string.Empty;
    }

    public List<string> GetFileLines(int id)
    {
        var filePath = GetFilePath(id);
        return [.. File.ReadAllLines(filePath)];
    }
}