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
    private static HashSet<string> StructsDefined = [];
    private static HashSet<string> PackagesDefined = [];
    
    public bool HasError { get; set; } = false;

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
                    if (!PackagesDefined.Contains(key))
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

        return root;
    }

    private (ASTNode?, string) ParseNode(ProgramNode root, ASTNode? lastNode)
    {
        var requires = string.Empty;
        while (GetTokenType() != TokenType.Eof)
        {
            var positionBefore = stream.Position;
            try
            {
                lastNode = ParseStatement();
                if (lastNode != null)
                {
                    if (lastNode is RequireNode requirement && !requirement.Satisfied)
                    {
                        requires = requirement.PackageName;
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

        while (GetTokenType() != TokenType.Eof)
        {
            var positionBefore = stream.Position;
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

    /// <summary>
    /// Recovers after a syntax error. If the current token cannot start a new
    /// statement (e.g. we are mid-expression), skip to the next newline so the
    /// next parse iteration begins on a fresh statement. Also handles the case
    /// where the parser has not advanced at all (preventing an infinite loop).
    /// </summary>
    private void RecoverAfterError(int positionBefore)
    {
        if (stream.Position <= positionBefore || !IsStatementStartToken())
        {
            // Advance past all tokens that belong to the same source line as the
            // statement that failed. Because the lexer doesn't always emit Newline
            // tokens, we use the token's Span.Line to find the line boundary.
            var statementLine = stream.At(positionBefore).Span.Line;
            while (GetTokenType() != TokenType.Eof && token.Span.Line == statementLine)
            {
                Next();
            }
        }
        // Current token is either on a new line or a valid statement-start token;
        // the outer loop will continue from here.
    }

    /// <summary>
    /// Returns true when the current token can legally begin a new statement,
    /// meaning we can safely resume parsing from this position after an error.
    /// </summary>
    private bool IsStatementStartToken() => GetTokenType() switch
    {
        TokenType.Operator or TokenType.Comma or TokenType.Colon
            or TokenType.RParen or TokenType.RBracket or TokenType.RBrace
            or TokenType.Dot or TokenType.Range or TokenType.Arrow => false,
        _ => true
    };

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
                return GetTokenName() is TokenName.KW_This or TokenName.KW_StaticSelf or TokenName.KW_Do;

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

    private void Rewind()
    {
        stream.Rewind();
        token = stream.Current();
    }

    private string GetTokenText() => token.Text;

    private TokenType GetTokenType() => token.Type;

    private TokenName GetTokenName() => token.Name;

    private bool IsValidTypeName() => GetTokenType() == TokenType.Typename || StructsDefined.Contains(GetTokenText());

    private int GetTypeName()
    {
        // expect a type name
        if (!IsValidTypeName())
        {
            throw new SyntaxError(GetErrorToken(), "Expected a type name in parameter type hint.");
        }

        // grab the type name
        var typeName = GetTokenText();
        Next();  // next token please

        return TypeRegistry.GetType(typeName);
    }

    private List<int> GetTypeNames()
    {
        var types = new List<int> { GetTypeName() };

        // support union types: type1|type2|...
        while (GetTokenName() == TokenName.Ops_BitwiseOr && IsValidTypeNameAhead())
        {
            Next();  // consume '|'
            types.Add(GetTypeName());
        }

        return types;
    }

    private bool IsValidTypeNameAhead()
    {
        var next = Peek();
        return next.Type == TokenType.Typename || StructsDefined.Contains(next.Text);
    }

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
        TokenName.Ops_Multiply or TokenName.Ops_Divide or TokenName.Ops_Modulus or TokenName.Ops_Exponent => true,
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
        TokenName.Ops_Assign or TokenName.Ops_AddAssign or TokenName.Ops_SubtractAssign or TokenName.Ops_MultiplyAssign or TokenName.Ops_DivideAssign or TokenName.Ops_ExponentAssign or TokenName.Ops_OrAssign or TokenName.Ops_AndAssign or TokenName.Ops_ModuloAssign or TokenName.Ops_BitwiseOrAssign or TokenName.Ops_BitwiseAndAssign or TokenName.Ops_BitwiseXorAssign or TokenName.Ops_BitwiseNotAssign or TokenName.Ops_BitwiseLeftShiftAssign or TokenName.Ops_BitwiseRightShiftAssign or TokenName.Ops_BitwiseUnsignedRightShiftAssign => true,
        _ => false,
    };

    private static bool IsAssignmentOperator(TokenName name) => name switch
    {
        TokenName.Ops_Assign or TokenName.Ops_AddAssign or TokenName.Ops_SubtractAssign or TokenName.Ops_MultiplyAssign or TokenName.Ops_DivideAssign or TokenName.Ops_ExponentAssign or TokenName.Ops_OrAssign or TokenName.Ops_AndAssign or TokenName.Ops_ModuloAssign or TokenName.Ops_BitwiseOrAssign or TokenName.Ops_BitwiseAndAssign or TokenName.Ops_BitwiseXorAssign or TokenName.Ops_BitwiseNotAssign or TokenName.Ops_BitwiseLeftShiftAssign or TokenName.Ops_BitwiseRightShiftAssign or TokenName.Ops_BitwiseUnsignedRightShiftAssign => true,
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