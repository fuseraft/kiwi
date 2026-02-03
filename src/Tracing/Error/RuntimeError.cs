using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class RuntimeError : KiwiError
{
    public List<string> Trace { get; }

    public RuntimeError(Token token, string? message, List<string> trace) 
        : base(token, "RuntimeError", message)
    {
        Trace = trace;
    }

    public RuntimeError(Token token, string type, string? message, List<string> trace) 
        : base(token, type, message)
    {
        Trace = trace;
    }
}