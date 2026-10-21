using kiwi.Parsing.AST;
using kiwi.Tracing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Parsing;

/// <summary>
/// The parser.
/// </summary>
/// <param name="rethrowErrors">A flag to control when to rethrow an exception.</param>
public partial class Parser(bool rethrowErrors = false)
{
    private readonly bool rethrow = rethrowErrors;
    private Token token = Token.Eof;
    private TokenStream stream = new([]);
    private Stack<string> structStack = new();
    private readonly List<bool> _generatorMarks = [];
    // The unsatisfied `require` each deferred token stream stopped at, and whether such a
    // require is kept as a statement rather than deferring its stream (see ParseTokenStreamCollection).
    private readonly Dictionary<TokenStream, RequireNode> lastRequire = [];
    private bool keepUnsatisfiedRequires = false;
    // > 0 while parsing call arguments, where `a, b = 1` is not a pack assignment.
    private int callArgDepth = 0;
    // Shared by every parse in the process (tasks parse on their own threads via eval);
    // used as sets, the byte value is unused.
    private static readonly System.Collections.Concurrent.ConcurrentDictionary<string, byte> StructsDefined = new();
    private static readonly System.Collections.Concurrent.ConcurrentDictionary<string, byte> PackagesDefined = new();
    
    public bool HasError { get; set; } = false;

    // Blocks opened by statements that failed to parse. Their `end` (and any else/elsif/
    // catch/finally) is still ahead and would be reported as a second, unrelated error.
    private int orphanedBlocks = 0;

    public ASTNode ParseTokenStreamCollection(List<TokenStream> streams)
    {
        ProgramNode root = new();
        ASTNode? lastNode = null;
        bool isRootTokenSet = false;
        Dictionary<string, List<TokenStream>> revisitStreams = [];

        foreach (var stream in streams)
        {
            this.stream = stream;
            token = stream.Current();

            if (!isRootTokenSet)
            {
                root.Token = token;
                root.IsEntryPoint = true;
                isRootTokenSet = true;
            }

            // check for requirements
            string requirement;
            (lastNode, requirement) = ParseNode(root, lastNode);

            if (string.IsNullOrEmpty(requirement))
            {
                List<string> packagesDefined = [];
                foreach (var (key, waitingStreams) in revisitStreams)
                {
                    if (!PackagesDefined.ContainsKey(key))
                    {
                        continue;
                    }

                    foreach (var waitingStream in waitingStreams)
                    {
                        this.stream = waitingStream;
                        token = waitingStream.Current();
                        (lastNode, requirement) = ParseNode(root, lastNode);
                    }

                    packagesDefined.Add(key);
                }

                foreach (var package in packagesDefined)
                {
                    revisitStreams.Remove(package);
                }

                continue;
            }
            
            if (!revisitStreams.TryGetValue(requirement, out List<TokenStream>? value))
            {
                value = [];
                revisitStreams.Add(requirement, value);
            }

            value.Add(stream);
        }

        // Streams still waiting require a package that was never defined. Parse the rest of
        // them anyway, keeping the `require`, so it fails at runtime with PackageUndefinedError
        // instead of the code silently never running.
        foreach (var waitingStreams in revisitStreams.Values)
        {
            foreach (var waitingStream in waitingStreams)
            {
                this.stream = waitingStream;
                token = waitingStream.Current();
                keepUnsatisfiedRequires = true;
                try
                {
                    // The unsatisfied require was consumed before the stream was set aside.
                    root.Statements.Add(new RequireNode(lastRequire[waitingStream].PackageName, false) { Token = lastRequire[waitingStream].Token });
                    (lastNode, _) = ParseNode(root, lastNode);
                }
                finally
                {
                    keepUnsatisfiedRequires = false;
                }
            }
        }

        return root;
    }

    private (ASTNode?, string) ParseNode(ProgramNode root, ASTNode? lastNode)
    {
        var requires = string.Empty;
        orphanedBlocks = 0;  // per token stream: a block can't close in another file
        while (GetTokenType() != TokenType.Eof)
        {
            var positionBefore = stream.Position;
            if (SkipOrphanedBlockToken())
            {
                continue;
            }

            try
            {
                lastNode = ParseStatement();
                if (lastNode != null)
                {
                    if (lastNode is RequireNode requirement && !requirement.Satisfied && !keepUnsatisfiedRequires)
                    {
                        requires = requirement.PackageName;
                        lastRequire[stream] = requirement;
                        break;
                    }

                    root.Statements.Add(lastNode);
                }
            }
            catch (KiwiError e)
            {
                if (rethrow)
                {
                    throw;
                }

                HasError = true;
                ErrorHandler.PrintError(e);
                RecoverAfterError(positionBefore);
            }
            catch (Exception e)
            {
                if (rethrow)
                {
                    throw;
                }

                HasError = true;
                ErrorHandler.PrintError(e, GetErrorToken(false));
                RecoverAfterError(positionBefore);
            }
        }

        return (lastNode, requires);
    }

    public ASTNode ParseTokenStream(TokenStream stream, bool isEntryPoint)
    {
        this.stream = stream;
        token = stream.Current();  // Set to beginning.

        ProgramNode root = new()
        {
            IsEntryPoint = isEntryPoint
        };
        orphanedBlocks = 0;

        while (GetTokenType() != TokenType.Eof)
        {
            var positionBefore = stream.Position;
            if (SkipOrphanedBlockToken())
            {
                continue;
            }

            try
            {
                var statement = ParseStatement();
                if (statement != null)
                {
                    root.Statements.Add(statement);
                }
            }
            catch (KiwiError e)
            {
                if (rethrow)
                {
                    throw;
                }

                HasError = true;
                ErrorHandler.PrintError(e);
                RecoverAfterError(positionBefore);
            }
            catch (Exception e)
            {
                if (rethrow)
                {
                    throw;
                }

                HasError = true;
                ErrorHandler.PrintError(e, GetErrorToken(false));
                RecoverAfterError(positionBefore);
            }
        }

        return root;
    }

    /// <summary>
    /// Parses a single expression from a token stream. Used for sub-parsing
    /// interpolation expressions.
    /// </summary>
    public ASTNode? ParseExpressionFromStream(TokenStream ts)
    {
        stream = ts;
        token = stream.Current();
        return ParseExpression();
    }

    /// <summary>
    /// Skips zero or more consecutive Newline tokens. Called inside delimiters
    /// (parens, brackets, braces) to allow multi-line expressions.
    /// </summary>
    private void SkipNewlines()
    {
        while (GetTokenType() == TokenType.Newline)
        {
            Next();
        }
    }

    // Type names (`bytes`, `list`, `string`, ...) are keywords, so using one as a name fails
    // wherever the parser wanted an identifier. These errors say so and suggest a fix, rather
    // than "Expected parameter name." or "Unexpected token in statement: Typename".
    private SyntaxError TypenameAsNameError(string use) =>
        TypenameAsNameError(GetErrorToken(), use);

    private SyntaxError TypenameAsNameError(string use, string fix) =>
        TypenameAsNameError(GetErrorToken(), use, fix);

    private static SyntaxError TypenameAsNameError(Token typeName, string use, string? fix = null) =>
        new(typeName, $"`{typeName.Text}` is a type name and can't be used as {use}. " +
                      (fix ?? $"Rename it, e.g. `{SuggestedRename(typeName.Text)}`."));

    private static string SuggestedRename(string typeName) => typeName switch
    {
        "bytes"     => "byte_count",
        "list"      => "items",
        "string"    => "text",
        "integer"   => "count",
        "float"     => "amount",
        "boolean"   => "flag",
        "hashmap"   => "entries",
        "date"      => "timestamp",
        "lambda"    => "callback",
        "generator" => "gen",
        "object"    => "obj",
        "pointer"   => "ptr",
        "any"       => "value",
        _           => $"{typeName}_value",
    };

    /// <summary>
    /// Recovers after a syntax error: skips to the next line so the next parse iteration
    /// begins on a fresh statement (always advancing, so it can't loop), and records any
    /// blocks the failed statement opened so their `end` isn't reported as a second error.
    /// </summary>
    private void RecoverAfterError(int positionBefore)
    {
        // Skip the rest of the line the error happened on. Resuming anywhere on it re-parses
        // the token that just failed and reports the same mistake again (`x = { bytes: 1 }`
        // failing at `bytes`, then once more treating `bytes` as a statement). Because the
        // lexer doesn't always emit Newline tokens, we use the token's Span.Line to find the
        // line boundary.
        var errorLine = token.Span.EndLine;
        while (GetTokenType() != TokenType.Eof && token.Span.Line <= errorLine)
        {
            Next();
        }

        orphanedBlocks += OpenedBlocks(positionBefore, stream.Position);
    }

    /// <summary>
    /// Net blocks (openers minus `end`s) among the tokens a failed statement consumed. Loops,
    /// lambdas and `on ... do` handlers are counted by their `do`; `abstract fn` has no body.
    /// </summary>
    private int OpenedBlocks(int from, int to)
    {
        int opened = 0;
        for (int i = from; i < to; i++)
        {
            switch (stream.At(i).Name)
            {
                case TokenName.KW_Method when i == from || stream.At(i - 1).Name != TokenName.KW_Abstract:
                case TokenName.KW_Do:
                case TokenName.KW_If:
                case TokenName.KW_Case:
                case TokenName.KW_Try:
                case TokenName.KW_Struct:
                case TokenName.KW_Package:
                case TokenName.KW_Interface:
                case TokenName.KW_Enum:
                    opened++;
                    break;
                case TokenName.KW_End:
                    opened--;
                    break;
            }
        }
        return Math.Max(0, opened);
    }

    /// <summary>
    /// Consumes the `end` of a block whose opening statement failed to parse, or skips the line
    /// of one of its else/elsif/catch/finally clauses; the statements between them still parse.
    /// </summary>
    private bool SkipOrphanedBlockToken()
    {
        if (orphanedBlocks == 0) return false;

        switch (GetTokenName())
        {
            case TokenName.KW_End:
                orphanedBlocks--;
                Next();
                return true;
            case TokenName.KW_Else:
            case TokenName.KW_ElseIf:
            case TokenName.KW_Catch:
            case TokenName.KW_Finally:
                var line = token.Span.Line;
                while (GetTokenType() != TokenType.Eof && token.Span.Line == line)
                {
                    Next();
                }
                return true;
        }
        return false;
    }

    private bool HasValue()
    {
        switch (GetTokenType())
        {
            case TokenType.Literal:
            case TokenType.String:
            case TokenType.Typename:
            case TokenType.Identifier:
            case TokenType.LParen:
            case TokenType.LBrace:
            case TokenType.LBracket:
            case TokenType.Lambda:  // 'with' lambda expression
            case TokenType.Interpolation:
                return true;

            case TokenType.Conditional:
                return GetTokenName() == TokenName.KW_Case;  // 'case' expression only

            case TokenType.Keyword:
                return GetTokenName() is TokenName.KW_This or TokenName.KW_StaticSelf or TokenName.KW_Do or TokenName.KW_Emit;

            case TokenType.Operator:
                var name = GetTokenName();
                return name is TokenName.Ops_Not or TokenName.Ops_Subtract or TokenName.Ops_BitwiseNot;

            default:
                return false;
        }
    }

    private Token Next()
    {
        stream.Next();
        token = stream.Current();
        return token;
    }

    private Token Peek() => stream.Peek();
    private Token PeekAt(int offset) => stream.At(stream.Position + offset);

    private Token Previous() => stream.Previous();

    private bool MatchType(TokenType expectedType)
    {
        if (GetTokenType() == expectedType)
        {
            Next();
            return true;
        }

        return false;
    }

    private bool MatchName(TokenName expectedName)
    {
        if (GetTokenName() == expectedName)
        {
            Next();
            return true;
        }

        return false;
    }

    private bool LookAhead(List<TokenName> names)
    {
        const int JumpThreshold = 10;

        int pos = stream.Position;
        int nameLength = names.Count;
        int jumps = pos;

        for (; pos + 1 < stream.Size; ++pos)
        {
            var tokenType = stream.At(pos).Type;

            // Stop scanning at scope-closing tokens - the target can't be in the
            // current expression if we've already left it.
            if (tokenType is TokenType.RBracket or TokenType.RParen or TokenType.RBrace)
            {
                return false;
            }

            int matches = 0;

            for (int i = 0; i < nameLength; ++i)
            {
                if (stream.At(pos + i).Name == names[i])
                {
                    ++matches;
                }
                else
                {
                    --matches;
                }
            }

            if (matches == nameLength)
            {
                jumps = pos - jumps;

                if (jumps > JumpThreshold)
                {
                    return false;
                }

                return true;
            }
        }

        return false;
    }

    /// <summary>
    /// Returns true when the token stream from the current position matches the
    /// pack-assignment pattern: [, identifier]+ followed immediately by '=' or '=&lt;'.
    /// Used to distinguish  a, b = [1,2]  from  for k, v in ...  and typed params.
    /// </summary>
    private bool IsPackAssignAhead()
    {
        int pos = stream.Position; // currently pointing at the first ','
        while (pos + 1 < stream.Size)
        {
            // Must be a comma
            if (stream.At(pos).Type != TokenType.Comma) return false;
            pos++;
            // Must be followed by an identifier
            if (stream.At(pos).Type != TokenType.Identifier) return false;
            pos++;
            // Now expect either another ',' (more LHS vars), '=', or '=<'
            var next = stream.At(pos);
            if (next.Name == TokenName.Ops_Assign || next.Name == TokenName.Ops_Unpack)
                return true;
            // Anything other than another comma → not a pack assignment
            if (next.Type != TokenType.Comma) return false;
        }
        return false;
    }

    private void Rewind()
    {
        stream.Rewind();
        token = stream.Current();
    }

    private string GetTokenText() => token.Text;

    private TokenType GetTokenType() => token.Type;

    private TokenName GetTokenName() => token.Name;

    private bool IsValidTypeName() => GetTokenType() == TokenType.Typename || StructsDefined.ContainsKey(GetTokenText());

    private int GetTypeName()
    {
        // expect a type name
        if (!IsValidTypeName())
        {
            throw new SyntaxError(GetErrorToken(), "Expected a type name in parameter type hint.");
        }

        // grab the type name (`none` is the type of `null`)
        var typeName = GetTokenName() == TokenName.Types_None ? "null" : GetTokenText();
        Next();  // next token please

        return TypeRegistry.GetType(typeName);
    }

    private List<int> GetTypeNames()
    {
        var types = new List<int> { GetTypeName() };

        // support nullable shorthand: typename? → typename|null
        if (GetTokenType() == TokenType.Question)
        {
            Next();  // consume '?'
            var nullType = TypeRegistry.GetType("null");
            if (!types.Contains(nullType))
                types.Add(nullType);
        }

        // support union types: type1|type2|...
        while (GetTokenName() == TokenName.Ops_BitwiseOr && IsValidTypeNameAhead())
        {
            Next();  // consume '|'
            types.Add(GetTypeName());

            // each member of a union can also be nullable
            if (GetTokenType() == TokenType.Question)
            {
                Next();  // consume '?'
                var nullType = TypeRegistry.GetType("null");
                if (!types.Contains(nullType))
                    types.Add(nullType);
            }
        }

        return types;
    }

    private bool IsValidTypeNameAhead()
    {
        var next = Peek();
        return next.Type == TokenType.Typename || StructsDefined.ContainsKey(next.Text);
    }

    /// <summary>
    /// Whether the binary operator at the current token continues the expression before it.
    /// An operator on the same line always does. One that starts a new line does too, so long
    /// expressions can break before `&&`, `+` and the like, except `-`, which at the start of
    /// a line begins a new statement (a negative number).
    /// </summary>
    private bool ContinuesExpression() =>
        Previous().Span.EndLine == token.Span.Line || token.Name != TokenName.Ops_Subtract;

    private bool IsUnaryOperator() => token.Name switch
    {
        TokenName.Ops_Not or TokenName.Ops_Subtract or TokenName.Ops_BitwiseNot => true,
        _ => false,
    };

    private bool IsEqualityOperator() => token.Name switch
    {
        TokenName.Ops_Equal or TokenName.Ops_NotEqual => true,
        _ => false,
    };

    private bool IsAdditiveOperator() => token.Name switch
    {
        TokenName.Ops_Add or TokenName.Ops_Subtract => true,
        _ => false,
    };

    private bool IsMultiplicativeOperator() => token.Name switch
    {
        TokenName.Ops_Multiply or TokenName.Ops_Divide or TokenName.Ops_IntDivide or TokenName.Ops_Modulus or TokenName.Ops_Exponent => true,
        _ => false,
    };

    private bool IsBitwiseOperator() => token.Name switch
    {
        TokenName.Ops_BitwiseLeftShift or TokenName.Ops_BitwiseRightShift or TokenName.Ops_BitwiseUnsignedRightShift => true,
        _ => false,
    };

    private bool IsComparisonOperator() => token.Name switch
    {
        TokenName.Ops_GreaterThan or TokenName.Ops_GreaterThanOrEqual or TokenName.Ops_LessThan or TokenName.Ops_LessThanOrEqual or TokenName.KW_In => true,
        _ => false,
    };

    private bool IsAssignmentOperator() => token.Name switch
    {
        TokenName.Ops_Assign or TokenName.Ops_AddAssign or TokenName.Ops_SubtractAssign or TokenName.Ops_MultiplyAssign or TokenName.Ops_DivideAssign or TokenName.Ops_IntDivideAssign or TokenName.Ops_ExponentAssign or TokenName.Ops_OrAssign or TokenName.Ops_AndAssign or TokenName.Ops_ModuloAssign or TokenName.Ops_BitwiseOrAssign or TokenName.Ops_BitwiseAndAssign or TokenName.Ops_BitwiseXorAssign or TokenName.Ops_BitwiseNotAssign or TokenName.Ops_BitwiseLeftShiftAssign or TokenName.Ops_BitwiseRightShiftAssign or TokenName.Ops_BitwiseUnsignedRightShiftAssign => true,
        _ => false,
    };

    private static bool IsAssignmentOperator(TokenName name) => name switch
    {
        TokenName.Ops_Assign or TokenName.Ops_AddAssign or TokenName.Ops_SubtractAssign or TokenName.Ops_MultiplyAssign or TokenName.Ops_DivideAssign or TokenName.Ops_IntDivideAssign or TokenName.Ops_ExponentAssign or TokenName.Ops_OrAssign or TokenName.Ops_AndAssign or TokenName.Ops_ModuloAssign or TokenName.Ops_BitwiseOrAssign or TokenName.Ops_BitwiseAndAssign or TokenName.Ops_BitwiseXorAssign or TokenName.Ops_BitwiseNotAssign or TokenName.Ops_BitwiseLeftShiftAssign or TokenName.Ops_BitwiseRightShiftAssign or TokenName.Ops_BitwiseUnsignedRightShiftAssign => true,
        _ => false,
    };

    private Token GetErrorToken(bool setHasError = true)
    {
        if (setHasError)
        {
            HasError = true;
        }

        if (GetTokenType() != TokenType.Eof)
        {
            return token;
        }

        Rewind();

        return GetErrorToken(setHasError);
    }

    /// <summary>
    /// Creates an <see cref="UnexpectedEndOfFileError"/> for an unclosed block.
    /// Points at the last real token consumed (closest to where EOF occurred)
    /// and includes the block opener's location in the message, giving the user
    /// two reference points to triangulate the missing or stray block delimiter.
    /// </summary>
    private UnexpectedEndOfFileError BlockEofError(Token openToken)
    {
        var errToken = GetErrorToken();
        var msg = $"Unclosed '{openToken.Text}' block. Block opened at line {openToken.Span.Line}.";
        return new UnexpectedEndOfFileError(errToken, msg);
    }

}