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
    private Stack<Dictionary<string, string>> mangledNameStack = new();
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
        try
        {
            while (GetTokenType() != TokenType.Eof)
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
        }
        catch (KiwiError e)
        {
            if (rethrow)
            {
                throw;
            }

            if (e is UnexpectedEndOfFileError && lastNode != null)
            {
                e.Token = lastNode.Token;
            }

            ErrorHandler.PrintError(e);
        }
        catch (Exception e)
        {
            if (rethrow)
            {
                throw;
            }

            ErrorHandler.PrintError(e, GetErrorToken());
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

        try
        {
            while (GetTokenType() != TokenType.Eof)
            {
                var statement = ParseStatement();
                if (statement != null)
                {
                    root.Statements.Add(statement);
                }
            }
        }
        catch (KiwiError e)
        {
            if (rethrow)
            {
                throw;
            }

            ErrorHandler.PrintError(e);
        }
        catch (Exception e)
        {
            if (rethrow)
            {
                throw;
            }

            ErrorHandler.PrintError(e, GetErrorToken());
        }

        return root;
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
                return true;

            case TokenType.Keyword:
                return GetTokenName() == TokenName.KW_This;

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
        TokenName.Ops_GreaterThan or TokenName.Ops_GreaterThanOrEqual or TokenName.Ops_LessThan or TokenName.Ops_LessThanOrEqual => true,
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

    private Dictionary<string, string> GetNameMap()
    {
        if (mangledNameStack.Count == 0)
        {
            return PushNameStack();
        }

        return mangledNameStack.Peek();
    }

    private bool HasName(string name)
    {
        Stack<Dictionary<string, string>> nameStack = new(mangledNameStack);

        while (nameStack.Count > 0)
        {
            var names = nameStack.Pop();
            if (names.ContainsKey(name))
            {
                return true;
            }
        }

        return false;
    }

    private string GetName(string name)
    {
        Stack<Dictionary<string, string>> nameStack = new(mangledNameStack);

        while (nameStack.Count > 0)
        {
            var names = nameStack.Pop();
            if (names.TryGetValue(name, out string? value))
            {
                return value ?? string.Empty;
            }
        }

        return string.Empty;
    }

    Dictionary<string, string> PushNameStack()
    {
        Dictionary<string, string> emptyMap = new();
        mangledNameStack.Push(emptyMap);
        return mangledNameStack.Peek();
    }

    private void PopNameStack()
    {
        if (!mangledNameStack.Any())
        {
            return;
        }

        mangledNameStack.Pop();
    }
}