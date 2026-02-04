using kiwi.Parsing;
using kiwi.Settings;
using kiwi.Tracing.Error;

namespace kiwi.Tracing;

public static class ErrorHandler
{
    public static void PrintError(KiwiError e)
    {
        List<string>? trace = e is RuntimeError error ? error.Trace : [];
        PrintError(e.Type, e.Message, e.Token, trace);
    }

    public static void PrintError(Exception e, Token token)
    {
        PrintError("An unexpected error occurred", e.Message, token);
    }

    public static void PrintError(string type, string message, Token token, List<string>? trace = null)
    {
        var span = token.Span;
        var filePath = FileRegistry.Instance.GetFilePath(span.File);

        List<string> lines = 
        [
            $"Timestamp: {DateTime.Now:yyyy-MM-dd hh:mm:ss tt}", 
            $"[{type}]: {message}"
        ];

        if (token.Type != TokenType.Default/* && token.Name != TokenName.Default*/)
        {
            lines.Add($"File: {filePath}:{span.Line}:{span.Pos}");
        }

        foreach (var line in lines.Skip(1))
        {
            PrintErrorText(line);
        }

        if (File.Exists(filePath))
        {
            PrintErrorLine(span);   
        }

        if (trace != null)
        {
            foreach (var traceLine in trace)
            {
                PrintErrorText(traceLine);
            }
        }

        lines.Add(string.Empty);

        File.AppendAllLines(Kiwi.Settings.CrashDumpPath, lines);
    }

    private static void PrintErrorLine(TokenSpan span)
    {
        var lines = FileRegistry.Instance.GetFileLines(span.File);
        if (lines.Count > span.Line && span.Line != 0)
        {
            var line = lines[span.Line - 1];
            PrintErrorText(line);
            for (int i = 0; i < line.Length; ++i)
            {
                if (i < span.Pos - 1)
                {
                    Console.Error.Write(' ');
                }
                else
                {
                    Console.Error.Write('^');
                }
            }
            PrintErrorText();
        }
    }

    public static void DumpCrashLog(Exception? e)
    {
        List<string> lines = [$"Timestamp: {DateTime.Now:yyyy-MM-dd hh:mm:ss tt}"];

        while (e != null)
        {
            lines.Add($"Message: {e.Message}");
            lines.Add($"Stack: {e.StackTrace}");
            e = e.InnerException;
        }

        lines.Add(string.Empty);

        File.AppendAllLines(Kiwi.Settings.CrashDumpPath, lines);
        PrintErrorText($"Please check the log: {Kiwi.Settings.CrashDumpPath}");

        Environment.Exit(1);
    }

    private static void PrintErrorText(string message = "")
    {
        var prevColor = Console.ForegroundColor;
        Console.ForegroundColor = ConsoleColor.Red;
        Console.Error.WriteLine(message);
        Console.ForegroundColor = prevColor;
    }
}