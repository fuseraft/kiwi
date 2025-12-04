using kiwi.Parsing;
using kiwi.Settings;
using kiwi.Tracing.Error;

namespace kiwi.Tracing;

public static class ErrorHandler
{
    public static void PrintError(KiwiError e)
    {
        PrintError(e.Type, e.Message, e.Token);
    }

    public static void PrintError(Exception e, Token token)
    {
        PrintError("An unexpected error occurred", e.Message, token);
    }

    public static void PrintError(string type, string message, Token token)
    {
        var span = token.Span;
        var filePath = FileRegistry.Instance.GetFilePath(span.File);

        List<string> lines = 
        [
            $"Timestamp: {DateTime.Now:yyyy-MM-dd hh:mm:ss tt}", 
            $"[{type}]: {message}"
        ];

        if (token.Type != TokenType.Default && token.Name != TokenName.Default)
        {
            lines.Add($"{filePath}:{span.Line}:{span.Pos}");
        }

        foreach (var line in lines.Skip(1))
        {
            Console.Error.WriteLine(line);
        }

        if (File.Exists(filePath))
        {
            PrintErrorLine(span);   
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
            Console.Error.WriteLine(line);
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
            Console.Error.WriteLine();
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
        Console.Error.WriteLine($"Please check the log: {Kiwi.Settings.CrashDumpPath}");

        Environment.Exit(1);
    }
}