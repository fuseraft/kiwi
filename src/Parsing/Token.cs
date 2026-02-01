using kiwi.Parsing;
using kiwi.Typing;

namespace kiwi.Parsing;

public class TokenStream(List<Token> tokens)
{
    private readonly List<Token> tokens = tokens;

    public bool Empty => Size == 0;
    public bool CanRead => Position < Size;
    public int Size => tokens.Count;
    public int Position { get; private set; } = 0;

    public Token At(int pos)
    {
        if (pos >= Size)
        {
            return Token.Eof;
        }

        return tokens[pos];
    }

    public Token Current()
    {
        if (Position >= Size)
        {
            return Token.Eof;
        }

        return tokens[Position];
    }

    public Token Previous()
    {
        if (Position - 1 < 0)
        {
            return tokens.First();
        }

        return tokens[Position - 1];
    }

    public void Rewind()
    {
        if (Position - 1 < 0)
        {
            return;  // TODO: is it okay to swallow this?
        }

        --Position;
    }

    public bool MatchType(TokenType type)
    {
        if (Current().Type == type)
        {
            Next();
            return true;
        }

        return false;
    }

    public bool MatchName(TokenName name)
    {
        if (Current().Name == name)
        {
            Next();
            return true;
        }

        return false;
    }

    public void Next()
    {
        if (Position < Size)
        {
            ++Position;

            if (Current().Type == TokenType.Comment)
            {
                Next();
            }
        }
    }

    public Token Peek()
    {
        if (Position + 1 < Size)
        {
            return tokens[Position + 1];
        }

        return Token.Eof;
    }
}

public struct Token(TokenType type, TokenName name, TokenSpan span, string text, Value value)
{
    private readonly string text = text;

    public readonly TokenType Type { get; } = type;
    public readonly TokenName Name { get; } = name;
    public TokenSpan Span { get; private set; } = span;
    public readonly string Text => text;
    public Value Value { get; set; } = value;

    public void SetSpan(TokenSpan span) { Span = span; }

    public readonly Token Clone()
    {
        return new Token(Type, Name, Span, Text, Value);
    }

    public static Token Eof => new(TokenType.Eof, TokenName.Default, new TokenSpan(), string.Empty, Value.Default);
}

public readonly struct TokenSpan(int file, int line, int pos)
{
    private readonly int pos = pos;

    public readonly int File { get; } = file;
    public readonly int Line { get; } = line;
    public readonly int Pos => pos;
}

public enum TokenType
{
    Default,
    Identifier,
    Comment,
    Comma,
    Keyword,
    Operator,
    Literal,
    Bytes,
    String,
    Newline,
    Escape,
    LParen,
    RParen,
    LBracket,
    RBracket,
    LBrace,
    RBrace,
    Conditional,
    Colon,
    Qualifier,
    Dot,
    Range,
    Typename,
    Lambda,
    Question,
    Arrow,
    Error,
    Eof
}