using System.Text;
using kiwi.Parsing.Keyword;
using kiwi.Tracing;
using kiwi.Typing;

namespace kiwi.Parsing;

public class Lexer : IDisposable
{
    private readonly Stream stream;
    private readonly int File;
    private readonly bool CloseOnDispose;
    private int LineNumber = 1;
    private int Position = 1;

    public Lexer(string path)
    {
        string code = System.IO.File.ReadAllText(path, System.Text.Encoding.UTF8);
        File = FileRegistry.Instance.RegisterFile(path);
        stream = new MemoryStream(System.Text.Encoding.UTF8.GetBytes(code));
        CloseOnDispose = true;
    }

    public Lexer(int fileId, string code)
    {
        File = fileId;
        stream = new MemoryStream(System.Text.Encoding.UTF8.GetBytes(code));
        CloseOnDispose = true;
    }

    /// <summary>
    /// Creates a lexer from a stream. Does NOT close the stream on dispose unless specified.
    /// </summary>
    /// <param name="dataStream">The input stream (e.g., Console.OpenStandardInput).</param>
    /// <param name="fileId">File ID for error reporting (-1 = stdin).</param>
    /// <param name="closeOnDispose">If true, stream is disposed when lexer is disposed.</param>
    public Lexer(Stream dataStream, int fileId = 0, bool closeOnDispose = true)
    {
        stream = dataStream ?? throw new ArgumentNullException(nameof(dataStream));
        File = fileId;
        CloseOnDispose = closeOnDispose;
    }

    public TokenStream GetTokenStream()
    {
        return new TokenStream(GetTokens());
    }

    public void Dispose()
    {
        /*
        Only dispose the underlying stream if we own it.
        This prevents closing external streams like Console.OpenStandardInput() or MemoryStreams passed from StdInRunner.
        */
        if (CloseOnDispose)
            stream?.Dispose();
    }

    private List<Token> GetTokens()
    {
        List<Token> tokens = [];
        stream.Seek(0, SeekOrigin.Begin);

        while (stream.CanRead && (tokens.Count == 0 || tokens.Last().Type != TokenType.Eof))
        {
            var token = GetToken();

            if(token.Type == TokenType.String && token.Text.Contains("${"))
            {
                TokenizeStringInterpolation(ref tokens, token);
            }
            else
            {
                tokens.Add(token);
            }
        }

        return tokens;
    }

    private Token GetToken()
    {
        SkipWhitespace();
        var span = CreateSpan();

        if (GetChar() is not char c)
        {
            return CreateToken(TokenType.Eof, span, string.Empty);
        }

        if (c == 'b')
        {
            char? next = PeekChar();
            if (next == '"' || next == '\'')
            {
                return TokenizeByteString(span, next);
            }
        }
        
        if (char.IsAsciiLetter(c) || c == '_')
        {
            return TokenizeKeywordOrIdentifier(span, c);
        }
        else if (char.IsDigit(c))
        {
            return TokenizeNumericLiteral(span, c);
        }
        else if (c == '"')
        {
            return TokenizeString(span);
        }
        else if (c == '\'')
        {
            return TokenizeRawString(span);
        }

        return TokenizeSymbol(span, c);
    }

    // Returns number of bytes needed for the next UTF-8 character based on first byte
    private static int Utf8SequenceLength(byte firstByte)
    {
        if (firstByte < 0x80) return 1;           // ASCII
        if ((firstByte & 0xE0) == 0xC0) return 2; // 2-byte
        if ((firstByte & 0xF0) == 0xE0) return 3; // 3-byte
        if ((firstByte & 0xF8) == 0xF0) return 4; // 4-byte
        return 1; // invalid, treat as single byte
    }

    // Decodes 1 to 4 bytes into a Unicode code point
    private static char? DecodeUtf8(byte[] bytes, int length)
    {
        if (length == 1)
        {
            return (char)bytes[0];
        }

        uint codePoint;
        switch (length)
        {
            case 2:
                codePoint = (uint)((bytes[0] & 0x1F) << 6 | (bytes[1] & 0x3F));
                break;
            case 3:
                codePoint = (uint)((bytes[0] & 0x0F) << 12 | (bytes[1] & 0x3F) << 6 | (bytes[2] & 0x3F));
                break;
            case 4:
                codePoint = (uint)((bytes[0] & 0x07) << 18 | (bytes[1] & 0x3F) << 12 | (bytes[2] & 0x3F) << 6 | (bytes[3] & 0x3F));
                break;
            default:
                return null; // invalid
        }

        if (codePoint > 0x10FFFF || (codePoint >= 0xD800 && codePoint <= 0xDFFF))
        {
            return null; // invalid surrogate or out of range
        }

        return (char)codePoint;
    }

    private char? PeekChar()
    {
        if (!stream.CanRead)
        {
            return null;
        }

        long origin = stream.Position;

        int firstByte = stream.ReadByte();
        if (firstByte == -1)
        {
            stream.Position = origin;
            return null;
        }

        int seqLen = Utf8SequenceLength((byte)firstByte);

        if (seqLen == 1)
        {
            stream.Position = origin;
            return (char)firstByte;
        }

        // Read the continuation bytes
        byte[] buffer = new byte[seqLen];
        buffer[0] = (byte)firstByte;

        for (int i = 1; i < seqLen; i++)
        {
            int b = stream.ReadByte();
            if (b == -1 || (b & 0xC0) != 0x80)
            {
                // Incomplete or invalid sequence...fallback to single byte
                stream.Position = origin;
                return (char)firstByte;
            }
            buffer[i] = (byte)b;
        }

        stream.Position = origin; // rewind

        var ch = DecodeUtf8(buffer, seqLen);
        return ch ?? (char)firstByte; // fallback on decode failure
    }

    private char? GetChar()
    {
        if (!stream.CanRead)
        {
            return null;
        }

        int firstByte = stream.ReadByte();
        if (firstByte == -1)
        {
            return null;
        }

        int seqLen = Utf8SequenceLength((byte)firstByte);

        if (seqLen == 1)
        {
            char cr = (char)firstByte;
            UpdatePositionAndLine(cr);
            return cr;
        }

        byte[] buffer = new byte[seqLen];
        buffer[0] = (byte)firstByte;

        bool valid = true;
        for (int i = 1; i < seqLen; i++)
        {
            int b = stream.ReadByte();
            if (b == -1 || (b & 0xC0) != 0x80)
            {
                valid = false;
                break;
            }
            buffer[i] = (byte)b;
        }

        char? decoded = valid ? DecodeUtf8(buffer, seqLen) : null;

        char c = decoded ?? (char)firstByte; // fallback to first byte on failure

        UpdatePositionAndLine(c);
        return c;
    }

    private void UpdatePositionAndLine(char c)
    {
        ++Position;

        if (c == '\r')
        {
            char? next = PeekChar();
            if (next == '\n')
            {
                GetChar(); // consume \n
            }
            c = '\n';
        }

        if (c == '\n')
        {
            LineNumber++;
            Position = 1;
        }
    }

    private void SkipWhitespace()
    {
        int byteValue;
        while ((byteValue = stream.ReadByte()) != -1)
        {
            var c = (char)byteValue;

            if (c == '\n' || c == '\r')
            {
                ++LineNumber;
                Position = 1;
            }
            else
            {
                ++Position;
            }

            if (!char.IsWhiteSpace(c) && c != '\n')
            {
                --stream.Position;
                --Position;
                break;
            }
        }
    }

    private static void TokenizeStringInterpolation(ref List<Token> tokens, Token token)
    {
        var span = token.Span;
        var text = token.Text;
        System.Text.StringBuilder sv = new();
        List<Token> interpTokens = [];
        var interpolate = false;

        for (int i = 0, braces = 0; i < text.Length; ++i)
        {
            var c = text[i];
            switch (c)
            {
                case '$':
                    {
                        if (i + 1 < text.Length && text[i + 1] == '{')
                        {
                            ++i; // skip "${"
                            ++braces;

                            var s = sv.ToString();
                            sv.Clear();

                            var st = CreateStringLiteralToken(span, s, Value.CreateString(s));
                            interpTokens.Add(st);
                            interpolate = true;
                        }
                    }
                    break;

                case '{':
                    if (braces > 0)
                    {
                        ++braces;
                    }

                    sv.Append(c);
                    break;

                case '}':
                    {
                        if (braces > 0)
                        {
                            --braces;
                        }

                        if (braces > 0 || !interpolate)
                        {
                            sv.Append(c);
                            break;
                        }

                        var code = sv.ToString();
                        Lexer lex = new(token.Span.File, code);
                        sv.Clear();

                        var tmpTokens = lex.GetTokens();
                        if (tmpTokens == null || tmpTokens.Count == 0)
                        {
                            // empty interpolation
                            break;
                        }

                        // string: "your name repeated ${repeater} time(s) is ${name * repeater}"
                        // interp: "your name repeated " + (repeater) + " time(s) is " + (name * repeater)

                        // if we aren't at the beginning of the string, concatenate.
                        if (interpTokens.Count > 0)
                        {
                            interpTokens.Add(CreateToken(TokenType.Operator, span, "+", TokenName.Ops_Add));
                        }

                        // wrap inner tokens in parentheses
                        interpTokens.Add(CreateToken(TokenType.LParen, span, "("));
                        foreach (var tmpToken in tmpTokens)
                        {
                            // PH: How does one do this? Can this check be removed?
                            if (tmpToken.Type == TokenType.Eof)
                            {
                                continue;
                            }

                            tmpToken.SetSpan(span);
                            interpTokens.Add(tmpToken);
                        }
                        interpTokens.Add(CreateToken(TokenType.RParen, span, ")"));

                        // if we aren't at the end of the string, concatenate
                        if (i + 1 < text.Length)
                        {
                            interpTokens.Add(CreateToken(TokenType.Operator, span, "+", TokenName.Ops_Add));
                        }

                        interpolate = false;
                    }
                    break;

                default:
                    sv.Append(c);
                    break;
            }
        }

        if (sv.Length > 0)
        {
            if (interpTokens.Count > 0 && interpTokens.Last().Name != TokenName.Ops_Add)
            {
                interpTokens.Add(CreateToken(TokenType.Operator, span, "+", TokenName.Ops_Add));
            }
            var s = sv.ToString();
            interpTokens.Add(CreateStringLiteralToken(span, s, Value.CreateString(s)));
        }

        tokens.AddRange(interpTokens);
    }

    private Token TokenizeSymbol(TokenSpan span, char c)
    {
        return c switch
        {
            '\n' => CreateToken(TokenType.Newline, span, Environment.NewLine),
            ',' => CreateToken(TokenType.Comma, span, ","),
            '@' => CreateToken(TokenType.Keyword, span, "@", TokenName.KW_This),
            ':' when PeekChar() == ':' => CreateToken(TokenType.Qualifier, span, $":{GetChar()}"),
            '.' when PeekChar() == '.' => CreateToken(TokenType.Range, span, $".{GetChar()}"),
            ':' => CreateToken(TokenType.Colon, span, ":"),
            '.' => CreateToken(TokenType.Dot, span, "."),
            '\\' => TokenizeEscaped(span),
            '(' or ')' => TokenizeParen(span, c),
            '[' or ']' => TokenizeBracket(span, c),
            '{' or '}' => TokenizeBrace(span, c),
            '#' => TokenizeComment(span),
            '/' when PeekChar() == '#' => TokenizeBlockComment(span),
            _ => TokenizeOperator(span, c),
        };
    }

    private Token TokenizeEscaped(TokenSpan span)
    {
        var c = GetChar();
        var text = c switch
        {
            '\\' => "\\",
            'b' => "\b",
            'r' => "\r",
            'n' => "\n",
            't' => "\t",
            'f' => "\f",
            '"' => "\"",
            _ => $"\\{c}",
        };
        return CreateToken(TokenType.Escape, span, text);
    }

    private Token TokenizeOperator(TokenSpan span, char c)
    {
        var sb = new System.Text.StringBuilder();
        sb.Append(c);

        char Consume()
        {
            var ch = GetChar();
            if (ch != null) sb.Append(ch.Value);
            return ch ?? '\0';
        }

        // op chars: ! + - * / % = < > | & ^ ~ ?
        while (true)
        {
            char? next = PeekChar();
            if (next == null)
            {
                break;
            }

            if (!"!+-*/%=<>&|^~?".Contains(next.Value))
            {
                break;
            }

            // null-coalesce
            if (sb.Length == 1 && sb[0] == '?' && next == '?')
            {
                Consume();
                continue;
            }

            // Don't allow more than 3 chars unless it's >>> or >>>=
            if (sb.Length >= 3 && !(sb.ToString() == ">>>" && next == '='))
            {
                break;
            }

            // Don't allow ?? followed by anything
            if (sb.ToString() == "??")
            {
                break;
            }

            Consume();
        }

        string text = sb.ToString();

        TokenName name = text switch
        {
            // 4-char
            ">>>=" => TokenName.Ops_BitwiseUnsignedRightShiftAssign,

            // 3-char
            ">>>" => TokenName.Ops_BitwiseUnsignedRightShift,
            "**=" => TokenName.Ops_ExponentAssign,
            "||=" => TokenName.Ops_OrAssign,
            "&&=" => TokenName.Ops_AndAssign,
            "<<=" => TokenName.Ops_BitwiseLeftShiftAssign,
            ">>=" => TokenName.Ops_BitwiseRightShiftAssign,

            // 2-char
            "=<" => TokenName.Ops_Unpack,
            "+=" => TokenName.Ops_AddAssign,
            "-=" => TokenName.Ops_SubtractAssign,
            "*=" => TokenName.Ops_MultiplyAssign,
            "/=" => TokenName.Ops_DivideAssign,
            "%=" => TokenName.Ops_ModuloAssign,
            "|=" => TokenName.Ops_BitwiseOrAssign,
            "&=" => TokenName.Ops_BitwiseAndAssign,
            "^=" => TokenName.Ops_BitwiseXorAssign,
            "~=" => TokenName.Ops_BitwiseNotAssign,
            "==" => TokenName.Ops_Equal,
            "!=" => TokenName.Ops_NotEqual,
            "<=" => TokenName.Ops_LessThanOrEqual,
            ">=" => TokenName.Ops_GreaterThanOrEqual,
            "<<" => TokenName.Ops_BitwiseLeftShift,
            ">>" => TokenName.Ops_BitwiseRightShift,
            "||" => TokenName.Ops_Or,
            "&&" => TokenName.Ops_And,
            "??" => TokenName.Ops_NullCoalesce,
            "**" => TokenName.Ops_Exponent,

            // 1-char
            "=" => TokenName.Ops_Assign,
            "!" => TokenName.Ops_Not,
            "+" => TokenName.Ops_Add,
            "-" => TokenName.Ops_Subtract,
            "*" => TokenName.Ops_Multiply,
            "/" => TokenName.Ops_Divide,
            "%" => TokenName.Ops_Modulus,
            "<" => TokenName.Ops_LessThan,
            ">" => TokenName.Ops_GreaterThan,
            "|" => TokenName.Ops_BitwiseOr,
            "&" => TokenName.Ops_BitwiseAnd,
            "^" => TokenName.Ops_BitwiseXor,
            "~" => TokenName.Ops_BitwiseNot,
            "?" => TokenName.Ops_Question, // ternary

            _ => TokenName.Default
        };

        // If unknown, treat as error or fallback
        if (name == TokenName.Default)
        {
            return CreateToken(TokenType.Error, span, text);
        } 
        else if (name == TokenName.Ops_Question)
        {
            // ternary
            return CreateToken(TokenType.Question, span, "?");
        }

        return CreateToken(TokenType.Operator, span, text, name);
    }

    private Token TokenizeByteString(TokenSpan span, char? quoteChar)
    {
        List<byte> bytes = [];
        bool escape = false;

        GetChar(); // consume opening quote

        StringBuilder sb = new();

        char? c;
        while ((c = PeekChar()) != null)
        {
            if (c == quoteChar)
            {
                GetChar(); // consume closing quote
                break;
            }
            else
            {
                sb.Append(c);
                GetChar();
            }
        }

        char[] chars = sb.ToString().ToCharArray();
        int pos = 0;

        while (pos < chars.Length)
        {
            c = chars[pos];
            if (escape)
            {
                escape = false;

                switch (c)
                {
                    case 'n':  bytes.Add((byte)'\n'); break;
                    case 'r':  bytes.Add((byte)'\r'); break;
                    case 't':  bytes.Add((byte)'\t'); break;
                    case '\\': bytes.Add((byte)'\\'); break;
                    case '"':  bytes.Add((byte)'"');  break;
                    case '\'': bytes.Add((byte)'\''); break;

                    case 'x':  // \xHH
                    case 'X':
                    {
                        string hex = "";
                        for (int i = 0; i < 2; i++)
                        {
                            char? h = chars[pos + 1];
                            if (h == null || !IsHexDigit(h.Value))
                            {
                                return CreateToken(TokenType.Error, span, "Invalid \\x escape in byte string");
                            }
                            hex += chars[++pos];
                        }
                        byte val = Convert.ToByte(hex, 16);
                        bytes.Add(val);
                        break;
                    }

                    default:
                        bytes.Add((byte)c); // literal char after \
                        break;
                }
            }
            else if (c == '\\')
            {
                escape = true;
                pos++;
            }
            else
            {
                // Only allow ASCII in byte strings (0-127)
                if (c > 127)
                {
                    return CreateToken(TokenType.Error, span, $"Non-ASCII character '{c}' in byte literal (use string and .to_bytes() for Unicode)");
                }
                bytes.Add((byte)c);
                pos++;
            }
        }

        var byteArray = bytes.ToArray();
        var value = Value.CreateBytes(byteArray);

        return new Token(TokenType.Bytes, TokenName.Default, span, $"b\"{string.Join("", bytes.Select(b => b.ToString("X2")))}\"", value);
    }

    private Token TokenizeString(TokenSpan span)
    {
        var text = string.Empty;
        bool escape = false;
        char? c;

        while ((c = PeekChar()) != null)
        {
            if (escape)
            {
                escape = false;
                GetChar();  // consume the escaped char

                switch (c)
                {
                    case 'n':  text += '\n'; break;
                    case 'r':  text += '\r'; break;
                    case 't':  text += '\t'; break;
                    case '\\': text += '\\'; break;
                    case '"':  text += '"';  break;
                    case 'b':  text += '\b'; break;
                    case 'f':  text += '\f'; break;
                    case 'u':  // \uXXXX
                    {
                        string hex = string.Empty;
                        bool valid = true;

                        for (int i = 0; i < 4; i++)
                        {
                            char? h = PeekChar();
                            if (h == null || !IsHexDigit(h.Value))
                            {
                                valid = false;
                                break;
                            }
                            hex += GetChar();
                        }

                        if (valid && hex.Length == 4)
                        {
                            int code = Convert.ToInt32(hex, 16);
                            text += (char)code;
                        }
                        else
                        {
                            // fallback: keep literal \uXXXX...
                            text += "\\u" + hex;
                        }
                        break;
                    }

                    case 'U':  // \UXXXXXXXX
                    {
                        string hex = string.Empty;
                        bool valid = true;

                        for (int i = 0; i < 8; i++)
                        {
                            char? h = PeekChar();
                            if (h == null || !IsHexDigit(h.Value))
                            {
                                valid = false;
                                break;
                            }
                            hex += GetChar();
                        }

                        if (valid && hex.Length == 8)
                        {
                            int code = Convert.ToInt32(hex, 16);
                            text += char.ConvertFromUtf32(code);  // handles surrogates
                        }
                        else
                        {
                            text += "\\U" + hex;
                        }
                        break;
                    }

                    default:
                        text += "\\" + c;
                        break;
                }
            }
            else if (c == '\\')
            {
                escape = true;
                GetChar();
            }
            else if (c == '"')
            {
                GetChar();  // Move past the closing quote
                break;      // End of string
            }
            else
            {
                text += c;
                GetChar();
            }
        }

        if (escape)
        {
            text += '\\';
        }

        return CreateStringLiteralToken(span, text, Value.CreateString(text));
    }

    private Token TokenizeRawString(TokenSpan span)
    {
        var text = string.Empty;
        char? c;

        while ((c = PeekChar()) != null)
        {
            if (c == '\'')
            {
                GetChar();  // Move past the closing quote
                break;      // End of string
            }
            else
            {
                text += c;
                GetChar();
            }
        }

        return CreateStringLiteralToken(span, text, Value.CreateString(text));
    }

    private static bool IsHexDigit(char c) => (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');

    private Token TokenizeBlockComment(TokenSpan span)
    {
        var text = string.Empty;
        GetChar(); // Skip '#'

        SkipWhitespace();
        char? ch;

        while ((ch = GetChar()) != null)
        {
            if (ch == '#' && PeekChar() == '/')
            {
                GetChar();
                break;
            }

            text += ch;
        }

        return CreateToken(TokenType.Comment, span, text);
    }

    private Token TokenizeComment(TokenSpan span)
    {
        var text = string.Empty;

        SkipWhitespace();
        char? ch;

        while ((ch = GetChar()) != null)
        {
            if (ch == '\n')
            {
                ++LineNumber;
                break;
            }

            text += ch;
        }

        return CreateToken(TokenType.Comment, span, text);
    }

    private static bool MatchChar(char? c, string within)
    {
        if (c == null)
        {
            return false;
        }

        return within.Contains((char)c);
    }

    private Token TokenizeNumericLiteral(TokenSpan span, char c)
    {
        var peek = PeekChar();
        
        if (c == '0' && MatchChar(peek, "xX"))
        {
            return TokenizeHexLiteral(span);
        }
        else if (c == '0' && MatchChar(peek, "bB"))
        {
            return TokenizeBinaryLiteral(span);
        }
        else if (c == '0' && MatchChar(peek, "oO"))
        {
            return TokenizeOctalLiteral(span);
        }
        
        return TokenizeLiteral(span, c);
    }

    private Token TokenizeHexLiteral(TokenSpan span)
    {
        var hexLiteral = string.Empty;
        GetChar();  // Move past 'x'

        char? ch;
        while ((ch = PeekChar()) != null && char.IsAsciiHexDigit((char)ch))
        {
            hexLiteral += GetChar();
        }

        var result = int.Parse(hexLiteral, System.Globalization.NumberStyles.HexNumber, System.Globalization.CultureInfo.InvariantCulture);

        return CreateLiteralToken(span, hexLiteral, Value.CreateInteger(result));
    }

    private Token TokenizeBinaryLiteral(TokenSpan span)
    {
        var binaryLiteral = string.Empty;
        GetChar();  // Move past 'b'

        char? ch;
        while ((ch = PeekChar()) != null && (ch == '0' || ch == '1'))
        {
            binaryLiteral += GetChar();
        }

        if (string.IsNullOrEmpty(binaryLiteral))
        {
            return CreateLiteralToken(span, binaryLiteral, Value.Default);
        }

        var result = int.Parse(binaryLiteral, System.Globalization.NumberStyles.BinaryNumber, System.Globalization.CultureInfo.InvariantCulture);

        return CreateLiteralToken(span, binaryLiteral, Value.CreateInteger(result));
    }

    private Token TokenizeOctalLiteral(TokenSpan span) 
    {
        var octalLiteral = string.Empty;
        GetChar();  // Move past 'o'

        char? ch;
        while ((ch = PeekChar()) != null && ch >= '0' && ch <= '7')
        {
            octalLiteral += GetChar();
        }

        if (string.IsNullOrEmpty(octalLiteral))
        {
            return CreateLiteralToken(span, octalLiteral, Value.Default);
        }

        var result = Convert.ToInt32(octalLiteral, 8);

        return CreateLiteralToken(span, octalLiteral, Value.CreateInteger(result));
    }

    private Token TokenizeLiteral(TokenSpan span, char c)
    {
        var text = string.Empty + c;
        var lastChar = '\0';
        char? ch;

        while ((ch = PeekChar()) != null && (char.IsDigit(ch.Value) || ch == '.' || ch == 'e' || ch == 'E'))
        {
            c = ch.Value;

            if (c == '.' && lastChar == '.')
            {
                text = text[..^1];
                --stream.Position;
                break;
            }

            text += c;
            lastChar = c;
            GetChar();
        }

        var dots = text.Count(c => c == '.');
        var es = text.ToLower().Count(c => c == 'e');

        if (dots > 1 || es > 1)
        {
            return CreateToken(TokenType.Error, span, text);
        }

        if (dots == 0)
        {
            return CreateLiteralToken(span, text, Value.CreateInteger(Convert.ToInt64(text)));
        }

        return CreateLiteralToken(span, text, Value.CreateFloat(Convert.ToDouble(text)));
    }

    private Token TokenizeKeywordOrIdentifier(TokenSpan span, char c)
    {
        var text = string.Empty + c;
        char? ch;

        while ((ch = PeekChar()) != null)
        {
            c = ch.Value;

            if (!char.IsAsciiLetterOrDigit(c) && c != '_')
            {
                break;
            }

            text += c;
            GetChar();
        }

        if (IsKeyword(text, out TokenName kw))
        {
            return CreateToken(TokenType.Keyword, span, text, kw);
        }
        else if (IsTypenameKeyword(text, out kw))
        {
            var typeName = CreateToken(TokenType.Typename, span, text, kw);
            typeName.Value = Value.CreateString(text);
            return typeName;
        }
        else if (IsLogicalKeyword(text, out kw))
        {
            return CreateToken(TokenType.Operator, span, text, kw);
        }
        else if (IsConditionalKeyword(text, out kw))
        {
            return CreateToken(TokenType.Conditional, span, text, kw);
        }
        else if (IsLiteralKeyword(text, out kw))
        {
            return text switch
            {
                "null" => CreateLiteralToken(span, text, Value.CreateNull(), kw),
                "true" => CreateLiteralToken(span, text, Value.CreateBoolean(true), kw),
                "false" => CreateLiteralToken(span, text, Value.CreateBoolean(false), kw),
                _ => CreateToken(TokenType.Error, span, text)
            };
        }
        else if (IsLambdaKeyword(text, out kw))
        {
            return CreateToken(TokenType.Lambda, span, text, kw);
        }
        else if (CoreBuiltin.IsBuiltin(text))
        {
            return TokenizeCoreBuiltin(span, text);
        }
        else if (CoreBuiltin.IsBuiltinMethod(text))
        {
            return TokenizeBuiltinMethod(span, text);
        }

        return CreateToken(TokenType.Identifier, span, text);
    }

    private static Token TokenizeBuiltinMethod(TokenSpan span, string builtin)
    {
        if (KiwiBuiltin.Map.TryGetValue(builtin, out TokenName name)) { }
        else if (ConsoleBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (EnvBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (FileIOBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (ListBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (CallableBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (MathBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (TaskBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (ChannelBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (SysBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (TimeBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (StdInBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (EncoderBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (SerializerBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (ReflectorBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (SocketBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (TlsSocketBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (HttpBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (CompressionBuiltin.Map.TryGetValue(builtin, out name)) { }

        return CreateToken(TokenType.Identifier, span, builtin, name);
    }

    private static Token TokenizeCoreBuiltin(TokenSpan span, string builtin)
    {
        if (CoreBuiltin.Map.TryGetValue(builtin, out TokenName name)) { }
        else if (ListBuiltin.Map.TryGetValue(builtin, out name)) { }
        else if (CallableBuiltin.Map.TryGetValue(builtin, out name)) { }

        return CreateToken(TokenType.Identifier, span, builtin, name);
    }

    private static TokenName GetOperatorName(string text)
    {
        return text switch
        {
            "=" => TokenName.Ops_Assign,
            "!" => TokenName.Ops_Not,
            "==" => TokenName.Ops_Equal,
            "!=" => TokenName.Ops_NotEqual,
            "<" => TokenName.Ops_LessThan,
            "<=" => TokenName.Ops_LessThanOrEqual,
            ">" => TokenName.Ops_GreaterThan,
            ">=" => TokenName.Ops_GreaterThanOrEqual,
            "+" => TokenName.Ops_Add,
            "+=" => TokenName.Ops_AddAssign,
            "-" => TokenName.Ops_Subtract,
            "-=" => TokenName.Ops_SubtractAssign,
            "*" => TokenName.Ops_Multiply,
            "*=" => TokenName.Ops_MultiplyAssign,
            "**" => TokenName.Ops_Exponent,
            "**=" => TokenName.Ops_ExponentAssign,
            "%" => TokenName.Ops_Modulus,
            "%=" => TokenName.Ops_ModuloAssign,
            "/" => TokenName.Ops_Divide,
            "/=" => TokenName.Ops_DivideAssign,
            "&&" => TokenName.Ops_And,
            "&&=" => TokenName.Ops_AndAssign,
            "||" => TokenName.Ops_Or,
            "||=" => TokenName.Ops_OrAssign,
            "^" => TokenName.Ops_BitwiseXor,
            "^=" => TokenName.Ops_BitwiseXorAssign,
            "~" => TokenName.Ops_BitwiseNot,
            "~=" => TokenName.Ops_BitwiseNotAssign,
            "|" => TokenName.Ops_BitwiseOr,
            "|=" => TokenName.Ops_BitwiseOrAssign,
            "&" => TokenName.Ops_BitwiseAnd,
            "&=" => TokenName.Ops_BitwiseAndAssign,
            "<<" => TokenName.Ops_BitwiseLeftShift,
            "<<=" => TokenName.Ops_BitwiseLeftShiftAssign,
            ">>" => TokenName.Ops_BitwiseRightShift,
            ">>=" => TokenName.Ops_BitwiseRightShiftAssign,
            ">>>" => TokenName.Ops_BitwiseUnsignedRightShift,
            ">>>=" => TokenName.Ops_BitwiseUnsignedRightShiftAssign,
            _ => TokenName.Default,
        };
    }

    private static bool IsTypenameKeyword(string text, out TokenName name)
    {
        name = TokenName.Default;

        switch (text)
        {
            case "integer":
            case "Integer":
                name = TokenName.Types_Integer;
                break;

            case "float":
            case "Float":
                name = TokenName.Types_Float;
                break;

            case "boolean":
            case "Boolean":
                name = TokenName.Types_Boolean;
                break;

            case "date":
            case "Date":
                name = TokenName.Types_Date;
                break;

            case "any":
            case "Any":
                name = TokenName.Types_Any;
                break;

            case "hashmap":
            case "Hashmap":
                name = TokenName.Types_Hashmap;
                break;

            case "lambda":
            case "Lambda":
                name = TokenName.Types_Lambda;
                break;

            case "string":
            case "String":
                name = TokenName.Types_String;
                break;

            case "list":
            case "List":
                name = TokenName.Types_List;
                break;

            case "object":
            case "Object":
                name = TokenName.Types_Object;
                break;

            case "none":
            case "None":
                name = TokenName.Types_None;
                break;

            case "pointer":
            case "Pointer":
                name = TokenName.Types_Pointer;
                break;

            case "bytes":
            case "Bytes":
                name = TokenName.Types_Bytes;
                break;
        }

        return name != TokenName.Default;
    }

    private static bool IsLambdaKeyword(string text, out TokenName name)
    {
        name = TokenName.Default;

        if ("with".Equals(text))
        {
            name = TokenName.KW_Lambda;
        }

        return name != TokenName.Default;
    }

    private static bool IsLiteralKeyword(string text, out TokenName name)
    {
        name = text switch
        {
            "null" => TokenName.KW_Null,
            "true" => TokenName.KW_True,
            "false" => TokenName.KW_False,
            _ => TokenName.Default,
        };

        return name != TokenName.Default;
    }

    private static bool IsLogicalKeyword(string text, out TokenName name)
    {
        name = text switch
        {
            "and" => TokenName.Ops_And,
            "or" => TokenName.Ops_Or,
            "not" => TokenName.Ops_Not,
            _ => TokenName.Default,
        };

        return name != TokenName.Default;
    }

    private static bool IsConditionalKeyword(string text, out TokenName name)
    {
        name = text switch
        {
            "if" => TokenName.KW_If,
            "elsif" => TokenName.KW_ElseIf,
            "else" => TokenName.KW_Else,
            "end" => TokenName.KW_End,
            "case" => TokenName.KW_Case,
            _ => TokenName.Default,
        };

        return name != TokenName.Default;
    }

    private static bool IsKeyword(string text, out TokenName name)
    {
        name = text switch
        {
            "abstract" => TokenName.KW_Abstract,
            "as" => TokenName.KW_As,
            "break" => TokenName.KW_Break,
            "catch" => TokenName.KW_Catch,
            "const" => TokenName.KW_Const,
            "do" => TokenName.KW_Do,
            "eprint" => TokenName.KW_EPrint,
            "eprintln" => TokenName.KW_EPrintLn,
            "emit" => TokenName.KW_Emit,
            "exit" => TokenName.KW_Exit,
            "export" => TokenName.KW_Export,
            "finally" => TokenName.KW_Finally,
            "for" => TokenName.KW_For,
            "import" => TokenName.KW_Import,
            "include" => TokenName.KW_Include,
            "in" => TokenName.KW_In,
            "interface" => TokenName.KW_Interface,
            "def" => TokenName.KW_Method,
            "fn" => TokenName.KW_Method,
            "package" => TokenName.KW_Package,
            "next" => TokenName.KW_Next,
            "off" => TokenName.KW_Off,
            "on" => TokenName.KW_On,
            "once" => TokenName.KW_Once,
            "override" => TokenName.KW_Override,
            "eval" => TokenName.KW_Eval, // eval is evil
            "pass" => TokenName.KW_Pass,
            "print" => TokenName.KW_Print,
            "println" => TokenName.KW_PrintLn,
            "printxy" => TokenName.KW_PrintXy,
            "private" => TokenName.KW_Private,
            "repeat" => TokenName.KW_Repeat,
            "return" => TokenName.KW_Return,
            "static" => TokenName.KW_Static,
            "struct" => TokenName.KW_Struct,
            "throw" => TokenName.KW_Throw,
            "to" => TokenName.KW_To,
            "try" => TokenName.KW_Try,
            "var" => TokenName.KW_Var,
            "when" => TokenName.KW_When,
            "while" => TokenName.KW_While,
            _ => TokenName.Default,
        };

        return name != TokenName.Default;
    }

    private static Token TokenizeParen(TokenSpan span, char c) => CreateToken(c == '(' ? TokenType.LParen : TokenType.RParen, span, $"{c}");

    private static Token TokenizeBracket(TokenSpan span, char c) => CreateToken(c == '[' ? TokenType.LBracket : TokenType.RBracket, span, $"{c}");

    private static Token TokenizeBrace(TokenSpan span, char c) => CreateToken(c == '{' ? TokenType.LBrace : TokenType.RBrace, span, $"{c}");

    private static Token CreateStringLiteralToken(TokenSpan span, string text, Value value, TokenName name = TokenName.Default) => new(TokenType.String, name, span, text, value);

    private static Token CreateLiteralToken(TokenSpan span, string text, Value value, TokenName name = TokenName.Default) => new(TokenType.Literal, name, span, text, value);

    private static Token CreateToken(TokenType type, TokenSpan span, string text, TokenName name = TokenName.Default) => new(type, name, span, text, Value.Default);

    private TokenSpan CreateSpan() => new(File, LineNumber, Position);
}