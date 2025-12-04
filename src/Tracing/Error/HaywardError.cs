using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class KiwiError : Exception
{
    public const string DefaultErrorType = "KiwiError";

    public KiwiError()
    {
    }

    public KiwiError(Token token, string? message) : this(token, DefaultErrorType, message)
    {
    }

    public KiwiError(Token token, string type, string? message) : base(message)
    {
        Token = token;
        Type = type;
    }

    public Token Token { get; set; }

    public string Type { get; set; } = DefaultErrorType;
}