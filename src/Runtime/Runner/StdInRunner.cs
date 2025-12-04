using kiwi.Parsing;
using kiwi.Tracing;
using kiwi.Tracing.Error;

namespace kiwi.Runtime.Runner;

/// <summary>
/// Runs a script from standard input (stdin), e.g., `cat script.kiwi | kiwi`.
/// </summary>
public class StdInRunner(Interpreter interpreter) : ScriptRunner(interpreter)
{
    /// <summary>
    /// Maximum size of input data streamed from stdin (default: 40 MB).
    /// </summary>
    public long MaxInputBufferSize { get; set; } = 40 * 1024 * 1024;

    /// <summary>
    /// Internal buffer size for reading stdin (4 KB).
    /// </summary>
    private const int BufferSize = 4096;

    /// <summary>
    /// Runs the standard input as a script stream.
    /// </summary>
    public override int Run(string script, List<string> args)
    {
        try
        {
            using var stdIn = Console.OpenStandardInput();
            using var memoryStream = new MemoryStream();

            long bytesRead = BufferStandardInput(stdIn, memoryStream, MaxInputBufferSize);
            if (bytesRead == 0)
                return SuccessReturnCode; // Empty input

            memoryStream.Position = 0;
            using var lexer = new Lexer(memoryStream, fileId: -1, closeOnDispose: false); // -1 = stdin
            return RunLexer(lexer);
        }
        catch (KiwiError e)
        {
            ErrorHandler.PrintError(e);
            return 1;
        }
        catch (Exception e)
        {
            ErrorHandler.DumpCrashLog(e);
            return 1;
        }
    }

    /// <summary>
    /// Efficiently buffers stdin into a MemoryStream with size limiting.
    /// </summary>
    /// <param name="stdIn">The input stream (Console.OpenStandardInput).</param>
    /// <param name="output">The destination MemoryStream.</param>
    /// <param name="maxBytes">Maximum bytes to read.</param>
    /// <returns>Total bytes read.</returns>
    /// <exception cref="KiwiError">Thrown when input exceeds max size.</exception>
    private static long BufferStandardInput(Stream stdIn, MemoryStream output, long maxBytes)
    {
        if (!stdIn.CanRead)
            throw new IOException("Standard input is not readable.");

        long totalBytes = 0;
        var buffer = new byte[BufferSize];

        while (true)
        {
            int bytesRead = stdIn.Read(buffer, 0, buffer.Length);
            if (bytesRead == 0)
                break;

            totalBytes += bytesRead;

            if (totalBytes > maxBytes)
                throw new IOException($"Input exceeded maximum size of {maxBytes} bytes.");

            output.Write(buffer, 0, bytesRead);
        }

        return totalBytes;
    }
}