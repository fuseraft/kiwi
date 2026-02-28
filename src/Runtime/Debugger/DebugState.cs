namespace kiwi.Runtime.Debugger;

public enum DebugMode { Running, StepIn, StepOver, StepOut }

public class DebugState
{
    private readonly List<(string File, int Line)> _breakpoints = [];

    public DebugMode Mode { get; set; } = DebugMode.StepIn;
    public int StepDepth { get; set; } = 0;
    public string LastFile { get; set; } = string.Empty;
    public int LastLine { get; set; } = -1;

    public IReadOnlyList<(string File, int Line)> Breakpoints => _breakpoints;

    public void AddBreakpoint(string file, int line)
    {
        _breakpoints.Add((file, line));
    }

    public bool RemoveBreakpoint(int index)
    {
        if (index < 1 || index > _breakpoints.Count)
        {
            return false;
        }

        _breakpoints.RemoveAt(index - 1);
        return true;
    }

    public bool HasBreakpoint(string file, int line)
    {
        foreach (var (f, l) in _breakpoints)
        {
            if (l == line && (string.IsNullOrEmpty(f) || f == file || file.EndsWith(f) || f.EndsWith(file)))
            {
                return true;
            }
        }

        return false;
    }
}
