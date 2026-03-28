namespace kiwi.Runtime.Runner;

/// <summary>
/// A simple interactive line editor with cursor movement, history,
/// and common readline-style key bindings.
/// Falls back to Console.ReadLine() when input is redirected (piped).
/// </summary>
internal sealed class LineEditor
{
    private readonly List<string> _history = [];
    private int _historyIndex;
    private string _savedInput = string.Empty;

    /// <summary>
    /// Reads a line of input, displaying <paramref name="prompt"/> first.
    /// Returns <c>null</c> on EOF (Ctrl+D on an empty line, or end of piped input).
    /// </summary>
    public string? ReadLine(string prompt, ConsoleColor promptColor = ConsoleColor.Green)
    {
        // Non-interactive (piped) input: skip the editor and use the basic readline.
        if (Console.IsInputRedirected)
        {
            Console.Write(prompt);
            return Console.ReadLine();
        }

        Console.ForegroundColor = promptColor;
        Console.Write(prompt);
        Console.ResetColor();
        int promptLen = prompt.Length;

        var buf = new List<char>();
        int pos = 0;

        _historyIndex = _history.Count;
        _savedInput = string.Empty;

        while (true)
        {
            ConsoleKeyInfo k;
            try { k = Console.ReadKey(intercept: true); }
            catch { return null; }

            switch (k.Key)
            {
                case ConsoleKey.Enter:
                    Console.WriteLine();
                    var line = new string([.. buf]);
                    if (!string.IsNullOrWhiteSpace(line))
                    {
                        _history.Add(line);
                    }
                    _historyIndex = _history.Count;
                    return line;

                case ConsoleKey.Backspace:
                    if (pos > 0)
                    {
                        buf.RemoveAt(pos - 1);
                        pos--;
                        Redraw(buf, pos, promptLen);
                    }
                    break;

                case ConsoleKey.Delete:
                    if (pos < buf.Count)
                    {
                        buf.RemoveAt(pos);
                        Redraw(buf, pos, promptLen);
                    }
                    break;

                case ConsoleKey.LeftArrow:
                    if (pos > 0)
                    {
                        SetCursor(promptLen + --pos);
                    }
                    break;

                case ConsoleKey.RightArrow:
                    if (pos < buf.Count)
                    {
                        SetCursor(promptLen + ++pos);
                    }
                    break;

                case ConsoleKey.Home:
                    pos = 0;
                    SetCursor(promptLen);
                    break;

                case ConsoleKey.End:
                    pos = buf.Count;
                    SetCursor(promptLen + pos);
                    break;

                case ConsoleKey.UpArrow:
                    if (_historyIndex > 0)
                    {
                        if (_historyIndex == _history.Count)
                        {
                            _savedInput = new string([.. buf]);
                        }
                        _historyIndex--;
                        Replace(buf, _history[_historyIndex], ref pos, promptLen);
                    }
                    break;

                case ConsoleKey.DownArrow:
                    if (_historyIndex < _history.Count)
                    {
                        _historyIndex++;
                        var next = _historyIndex == _history.Count
                            ? _savedInput
                            : _history[_historyIndex];
                        Replace(buf, next, ref pos, promptLen);
                    }
                    break;

                default:
                    if ((k.Modifiers & ConsoleModifiers.Control) != 0)
                    {
                        HandleControl(k.Key, buf, ref pos, promptLen);
                    }
                    else if (k.KeyChar >= 32) // printable character
                    {
                        buf.Insert(pos, k.KeyChar);
                        pos++;
                        if (pos == buf.Count)
                        {
                            Console.Write(k.KeyChar); // fast path: append at end
                        }
                        else
                        {
                            Redraw(buf, pos, promptLen);
                        }
                    }
                    break;
            }
        }
    }

    private void HandleControl(ConsoleKey key, List<char> buf, ref int pos, int promptLen)
    {
        switch (key)
        {
            case ConsoleKey.A: // Ctrl+A: start of line
                pos = 0;
                SetCursor(promptLen);
                break;

            case ConsoleKey.E: // Ctrl+E: end of line
                pos = buf.Count;
                SetCursor(promptLen + pos);
                break;

            case ConsoleKey.K: // Ctrl+K: kill to end of line
                buf.RemoveRange(pos, buf.Count - pos);
                Redraw(buf, pos, promptLen);
                break;

            case ConsoleKey.U: // Ctrl+U: kill to start of line
                buf.RemoveRange(0, pos);
                pos = 0;
                Redraw(buf, 0, promptLen);
                break;

            case ConsoleKey.W: // Ctrl+W: delete word before cursor
                if (pos > 0)
                {
                    int start = pos - 1;
                    while (start > 0 && buf[start - 1] != ' ')
                    {
                        start--;
                    }
                    buf.RemoveRange(start, pos - start);
                    pos = start;
                    Redraw(buf, pos, promptLen);
                }
                break;

            case ConsoleKey.D: // Ctrl+D: EOF on empty line
                if (buf.Count == 0)
                {
                    Console.WriteLine();
                    // Signal EOF by returning (handled via null check in ReadLine caller).
                    // We throw to unwind without further processing.
                    throw new EndOfStreamException();
                }
                break;
        }
    }

    private static void SetCursor(int col)
    {
        try { Console.CursorLeft = col; }
        catch (ArgumentOutOfRangeException) { }
    }

    /// <summary>
    /// Redraws the buffer from the start of the input area and repositions the cursor.
    /// Uses ANSI erase-to-end-of-line to clear any leftover characters.
    /// </summary>
    private static void Redraw(List<char> buf, int pos, int promptLen)
    {
        SetCursor(promptLen);
        Console.Write(new string([.. buf]));
        Console.Write("\x1b[0K"); // erase to end of line
        SetCursor(promptLen + pos);
    }

    /// <summary>
    /// Replaces the current buffer with <paramref name="text"/> and redraws.
    /// </summary>
    private static void Replace(List<char> buf, string text, ref int pos, int promptLen)
    {
        buf.Clear();
        buf.AddRange(text);
        pos = text.Length;
        Redraw(buf, pos, promptLen);
    }
}
