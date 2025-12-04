using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class StdinBuiltinHandler
{
    private static readonly Lazy<Stream> _rawStream = new(
        () => Console.OpenStandardInput(),
        isThreadSafe: true);

    private static readonly Lazy<StreamReader> _textReader = new(
        () => new StreamReader(_rawStream.Value, leaveOpen: true),
        isThreadSafe: true);

    private static Stream RawStream => _rawStream.Value;
    private static StreamReader TextReader => _textReader.Value;

    // Track if we've consumed everything via read() or read_bytes()
    private static volatile bool _fullyConsumed = false;

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Stdin_Read       => Read(token, args),
            TokenName.Builtin_Stdin_ReadLine   => ReadLine(token, args),
            TokenName.Builtin_Stdin_Lines      => Lines(token, args),
            TokenName.Builtin_Stdin_Empty      => Empty(token, args),
            TokenName.Builtin_Stdin_ReadBytes  => ReadBytes(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Read(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, StdInBuiltin.Read, 0, args.Count);
        _fullyConsumed = true;
        return Value.CreateString(TextReader.ReadToEnd());
    }

    private static Value ReadLine(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, StdInBuiltin.ReadLine, 0, args.Count);

        if (_fullyConsumed)
        {
            return Value.CreateString(string.Empty);
        }

        var line = TextReader.ReadLine();
        if (line == null)
        {
            _fullyConsumed = true;
        }

        return Value.CreateString(line ?? string.Empty);
    }

    private static Value Lines(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, StdInBuiltin.Lines, 0, args.Count);

        List<Value> list = [];
        Value line;
        while ((line = ReadLine(token, [])) is { } l && l.Type != Typing.ValueType.None)
        {
            list.Add(l);
        }
        _fullyConsumed = true;
        return Value.CreateList();
    }

    private static Value Empty(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, StdInBuiltin.Empty, 0, args.Count);

        if (_fullyConsumed)
        {
            return Value.CreateBoolean(true);
        }

        // Peek one character to test if stream is at EOF
        var peek = TextReader.Peek();
        return Value.CreateBoolean(peek == -1);
    }

    private static Value ReadBytes(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, StdInBuiltin.ReadBytes, 0, args.Count);

        _fullyConsumed = true;

        using var ms = new MemoryStream();
        RawStream.CopyTo(ms);
        var bytes = ms.ToArray();

        List<Value> list = [];
        foreach (var b in bytes)
        {
            list.Add(Value.CreateInteger(b));
        }

        return Value.CreateList(list);
    }
}