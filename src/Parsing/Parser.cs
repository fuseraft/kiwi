using System.Security.Cryptography.X509Certificates;
using kiwi.Parsing.AST;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Parsing;

public partial class Parser
{
    private ASTNode? ParseConditional()
    {
        if (GetTokenName() == TokenName.KW_If)
        {
            return ParseIf();
        }
        else if (GetTokenName() == TokenName.KW_Case)
        {
            return ParseCase();
        }

        throw new SyntaxError(GetErrorToken(), "Expected if-statement or case-statement.");
    }

    private ASTNode? ParseError()
    {
        ThrowNode node = new()
        {
            ErrorValue = new LiteralNode(Value.CreateString(token.Text))
        };

        Next();
        return node;
    }

    private ASTNode? ParseKeyword()
    {
        switch (GetTokenName())
        {
            case TokenName.KW_Const:
                return ParseConstAssignment();

            case TokenName.KW_Do:
                return ParseDo();

            case TokenName.KW_On:
            case TokenName.KW_Once:
                return ParseEventHandler();

            case TokenName.KW_Emit:
                return ParseEmit();

            case TokenName.KW_Off:
                return ParseOff();

            case TokenName.KW_PrintLn:
            case TokenName.KW_Print:
            case TokenName.KW_EPrintLn:
            case TokenName.KW_EPrint:
                return ParsePrint();

            case TokenName.KW_PrintXy:
                return ParsePrintXy();

            case TokenName.KW_For:
                return ParseForLoop();

            case TokenName.KW_While:
                return ParseWhileLoop();

            case TokenName.KW_This:
                return ParseIdentifier(false, false);

            case TokenName.KW_Repeat:
                return ParseRepeatLoop();

            case TokenName.KW_Try:
                return ParseTry();

            case TokenName.KW_Return:
                return ParseReturn();

            case TokenName.KW_Throw:
                return ParseThrow();

            case TokenName.KW_Exit:
                return ParseExit();

            case TokenName.KW_Eval:
                return ParseEval();

            case TokenName.KW_Next:
                return ParseNext();

            case TokenName.KW_Break:
                return ParseBreak();

            case TokenName.KW_Pass:
                Next();
                return new NoOpNode();

            case TokenName.KW_Package:
                return ParsePackage();

            case TokenName.KW_Import:
                return ParseImport();

            case TokenName.KW_Include:
                return ParseInclude();

            case TokenName.KW_Export:
                return ParseExport();

            case TokenName.KW_Struct:
                return ParseStruct();

            case TokenName.KW_Interface:
                return ParseInterface();

            case TokenName.KW_Method:
                return ParseFunction();

            case TokenName.KW_Var:
                return ParseVar();

            default:
                throw new SyntaxError(GetErrorToken(), $"Unexpected keyword '{token.Text}'.");
        }
    }

    private OffNode ParseOff()
    {
        MatchName(TokenName.KW_Off);

        // off "event-name"
        if (GetTokenType() != TokenType.String)
        {
            throw new SyntaxError(GetErrorToken(), "Expected string-literal for event name.");
        }

        var eventName = ParseExpression() ?? throw new SyntaxError(GetErrorToken(), "Expected event name for 'off'.");
        ASTNode? callback = null;

        if (MatchType(TokenType.LParen))
        {
            // off "event-name" (handler)
            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected an event handler.");
            }

            callback = ParseIdentifier(false, false);

            if (!MatchType(TokenType.RParen))
            {
                throw new SyntaxError(GetErrorToken(), "Expected ')' in event registration.");
            }
        }

        return new OffNode(eventName, callback);
    }

    private EmitNode ParseEmit()
    {
        MatchName(TokenName.KW_Emit);

        // emit "event-name" [ (arguments...) ]
        if (GetTokenType() != TokenType.String)
        {
            throw new SyntaxError(GetErrorToken(), "Expected string-literal for event name.");
        }

        var eventName = ParseExpression() ?? throw new SyntaxError(GetErrorToken(), "Expected event name for 'emit'.");

        List<ASTNode?> arguments = [];
        if (GetTokenType() == TokenType.LParen)
        {
            arguments = CollectCallArguments();
        }

        return new EmitNode(eventName, arguments);
    }

    private ASTNode ParseEventHandler()
    {
        var eventType = GetTokenName();
        Next(); // Consume 'on' or 'once'

        /*
        on "event-name" [ with (variables...) ] do
          [ statements ]
        end

        once "event-name" [ with (variables...) ] do
          [ statements ]
        end

        on "event-name" (lambda)
        */

        if (GetTokenType() != TokenType.String)
        {
            throw new SyntaxError(GetErrorToken(), "Expected a string-literal for event name.");
        }

        Token t = token.Clone();
        ASTNode eventName = ParseExpression() ?? throw new SyntaxError(GetErrorToken(), "Expected an event name.");
        ASTNode? callback = null;

        if (GetTokenName() == TokenName.KW_Lambda)
        {
            // on "event-name" with (variables...) do [ statements ] end
            callback = ParseLambda();
        }
        else if (MatchType(TokenType.LParen))
        {
            // on "event-name" (lambda)
            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected an event handler.");
            }

            callback = ParseIdentifier(false, false);

            if (!MatchType(TokenType.RParen))
            {
                throw new SyntaxError(GetErrorToken(), "Expected ')' in event registration.");
            }
        }
        else if (GetTokenName() == TokenName.KW_Do)
        {
            // on "event-name" do [ statements ] end
            Next(); // Consume 'do'

            List<ASTNode?> body = [];
            while (GetTokenName() != TokenName.KW_End)
            {
                var stmt = ParseStatement();
                if (stmt != null)
                {
                    body.Add(stmt);
                }
            }

            Next();  // Consume 'end'

            callback = new LambdaNode
            {
                Parameters = [],
                Body = body,
                TypeHints = [],
                ReturnTypeHint = 0,
                Token = t
            };
        }

        if (callback == null)
        {
            throw new SyntaxError(GetErrorToken(), "Expected an event handler.");
        }

        callback.Token = t;
        
        if (eventType == TokenName.KW_On)
        {
            return new OnNode(eventName, callback);
        }
        else if (eventType == TokenName.KW_Once)
        {
            return new OnceNode(eventName, callback);
        }

        throw new SyntaxError(GetErrorToken(), "Expected an event handler.");
    }

    private DoNode? ParseDo()
    {
        MatchName(TokenName.KW_Do);  // Consume 'do'

        /*
        do 
          [ statements ]
        [ when when_condition ];
        */

        List<ASTNode?> body = [];
        ASTNode? condition = null;
        while (GetTokenName() != TokenName.KW_End)
        {
            // if we find `when`, grab the condition and check for block separator.
            if (MatchName(TokenName.KW_When))
            {
                if (!HasValue())
                {
                    throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
                }

                condition = ParseExpression();

                if (GetTokenName() != TokenName.KW_End)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected block separator at end of 'do'.");
                }

                continue;
            }

            var stmt = ParseStatement();
            if (stmt != null)
            {
                body.Add(stmt);
            }
        }

        Next();  // Consume 'end'

        return new DoNode
        {
            Condition = condition,
            Body = body
        };
    }

    private ASTNode? ParseStatement()
    {
        var nodeToken = token.Clone();
        ASTNode? node;

        switch (nodeToken.Type)
        {
            case TokenType.Comment:
                node = ParseComment();
                break;

            case TokenType.Comma:
                MatchType(TokenType.Comma);
                return null;

            case TokenType.Error:
                node = ParseError();
                break;

            case TokenType.Keyword:
                node = ParseKeyword();
                break;

            case TokenType.Conditional:
                node = ParseConditional();
                break;

            case TokenType.LBrace:
            case TokenType.LBracket:
            case TokenType.LParen:
            case TokenType.Literal:
            case TokenType.Operator:
            case TokenType.Identifier:
            case TokenType.String:
            case TokenType.Bytes:
                node = ParseExpression();
                break;

            case TokenType.Newline:
                return null;

            default:
                throw new TokenStreamError(GetErrorToken(), $"Unexpected token in statement: {Enum.GetName(nodeToken.Type)}: `{nodeToken.Text}`");
        }

        if (node != null)
        {
            node.Token = nodeToken;
            return node;
        }

        return node;
    }

    private ASTNode? ParseComment()
    {
        MatchType(TokenType.Comment);
        return null;
    }

    private StructNode? ParseStruct()
    {
        MatchName(TokenName.KW_Struct);

        if (GetTokenType() != TokenType.Identifier)
        {
            throw new SyntaxError(GetErrorToken(), "Expected identifier for struct name.");
        }

        var structName = token.Text;
        Next();

        var baseStruct = string.Empty;

        // Extends
        if (MatchName(TokenName.Ops_LessThan))
        {
            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected identifier for base struct name.");
            }

            baseStruct = token.Text;
            Next();
        }

        List<string> interfaces = [];
        if (MatchType(TokenType.Colon))
        {
            while (GetTokenType() != TokenType.Keyword)
            {
                if (GetTokenType() == TokenType.Identifier)
                {
                    interfaces.Add(token.Text);
                }
                Next();
            }
        }
        
        // Push to the stack for operator overload detection.
        structStack.Push(structName);

        // Eagerly mark as defined so that the type can be used in typehints within the struct definition
        structsDefined.Add(structName);

        List<ASTNode?> methods = [];
        bool isStatic = false, isPrivate = false;
        
        while (GetTokenName() != TokenName.KW_End)
        {
            if (MatchName(TokenName.KW_Static))
            {
                isStatic = true;
                continue;
            }
            else if (MatchName(TokenName.KW_Private))
            {
                isPrivate = true;
                continue;
            }

            var statement = ParseStatement() ?? throw new SyntaxError(GetErrorToken(), "Expected a function.");
            if (statement.Type == ASTNodeType.Function)
            {
                var func = (FunctionNode)statement;
                func.IsPrivate = isPrivate;
                func.IsStatic = isStatic;

                methods.Add(func);
            }

            isStatic = false;
            isPrivate = false;
        }

        Next();  // Consume 'end'

        // Outside the struct definition
        structStack.Pop();

        return new StructNode(structName, baseStruct, interfaces, methods);
    }

    private ASTNode? ParseInterface()
    {
        // WIP:
        // List<ASTNode?> methods;
        // while (GetTokenName() != TokenName.KW_End) {
        //   if (GetTokenName() == TokenName.KW_Method) {}
        // }
        return null;
    }

    private VariableNode? ParseVar()
    {
        MatchType(TokenType.Keyword);  // consume 'var'

        /*
        var (
          f: float = 0.5,            # type-hint with initializer 
          s = "string",              # regular variable declaration
          n,                         # uninitialized variable defaults to null
          b: boolean,                # type-hint without initializer (`boolean` defaults to false)
          m: list = [[0] * 5] * 5    # a 5x5 matrix for funsies
        )
        */

        // syntax check
        if (GetTokenType() != TokenType.LParen)
        {
            throw new SyntaxError(GetErrorToken(), "Expected '(' after 'var'.");
        }
        Next();  // next token please

        // create a mangler (for preventing name collisions)
        var mangler = $"_{Guid.NewGuid().ToString().Substring(0, 8)}_";

        // get the mangled name map, we need to update this as names are mangled.
        var mangledNames = GetNameMap();

        // a container for variables
        List<KeyValuePair<string, ASTNode?>> variables = [];

        // a container for type-hints, mapped to variable names.
        Dictionary<string, int> typeHints = [];

        // collect variable declarations
        while (GetTokenType() != TokenType.RParen)
        {
            // expect an identifier
            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected variable name.");
            }

            // grab the variable name directly from token text
            var varName = token.Text;

            // check if the name is already mangled
            if (mangledNames.ContainsKey(varName))
            {
                throw new SyntaxError(GetErrorToken(), $"The variable name `{varName}` is already declared.");
            }

            // mangle the variable name
            var mangledName = mangler + varName;

            // register to the mangled names map
            mangledNames[varName] = mangledName;

            ASTNode? defaultValue = null;
            Next();  // next token please

            // check for a type-hint after a ':'
            if (MatchType(TokenType.Colon))
            {
                // register type-hint for the variable
                typeHints[mangledName] = GetTypeName();
            }

            // check for default value
            if (GetTokenType() == TokenType.Operator &&
                GetTokenName() == TokenName.Ops_Assign)
            {
                Next();  // consume '='
                defaultValue = ParseExpression();
            }

            // add to variables container
            variables.Add(new KeyValuePair<string, ASTNode?>(mangledName, defaultValue));

            // check for the next variable or the end of the variable declaration
            if (GetTokenType() == TokenType.Comma)
            {
                Next();  // next token please
            }
            else if (GetTokenType() != TokenType.RParen)
            {
                // if it's not a comma and it's not a ')', let the dev know there is a syntax error.
                throw new SyntaxError(GetErrorToken(), "Expected ',' or ')' in variable declaration list.");
            }
        }

        Next();  // consume ')'

        // create and return the node
        return new VariableNode
        {
            Variables = variables,
            TypeHints = typeHints
        };
    }

    private FunctionNode? ParseFunction()
    {
        MatchType(TokenType.Keyword);  // Consume 'fn'

        var isTypeName = GetTokenType() == TokenType.Typename;
        var isOperator = GetTokenType() == TokenType.Operator;

        if (GetTokenType() != TokenType.Identifier && !isTypeName && !isOperator)
        {
            throw new SyntaxError(GetErrorToken(), "Expected identifier after 'fn'.");
        }

        if (isOperator && structStack.Count == 0)
        {
            throw new SyntaxError(GetErrorToken(), "Global operator overloading is not supported.");
        }

        string functionName = token.Text;
        Next();

        var mangler = $"_{Guid.NewGuid().ToString()[..8]}_";

        // Parse parameters
        List<KeyValuePair<string, ASTNode?>> parameters = [];
        Dictionary<string, int> typeHints = [];
        int returnTypeHint = 0; // 0 is any

        if (isTypeName && GetTokenType() != TokenType.LParen)
        {
            throw new SyntaxError(GetErrorToken(), $"Expected '(' after the identifier `{functionName}`.");
        }

        var mangledNames = PushNameStack();

        if (GetTokenType() == TokenType.LParen)
        {
            Next();  // Consume '('

            while (GetTokenType() != TokenType.RParen)
            {
                if (GetTokenType() != TokenType.Identifier)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected parameter name.");
                }

                var paramName = token.Text;

                if (mangledNames.ContainsKey(paramName))
                {
                    throw new SyntaxError(GetErrorToken(), $"The parameter name `{paramName}` is already used.");
                }

                var mangledName = mangler + paramName;
                mangledNames[paramName] = mangledName;
                ASTNode? defaultValue = null;
                Next();

                if (MatchType(TokenType.Colon))
                {
                    typeHints[mangledName] = GetTypeName();
                }

                // Check for default value
                if (GetTokenType() == TokenType.Operator &&
                    GetTokenName() == TokenName.Ops_Assign)
                {
                    Next();  // Consume '='
                    defaultValue = ParseExpression();
                }

                parameters.Add(new KeyValuePair<string, ASTNode?>(mangledName, defaultValue));

                if (GetTokenType() == TokenType.Comma)
                {
                    Next();
                }
                else if (GetTokenType() != TokenType.RParen)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected ',' or ')' in parameter list.");
                }
            }

            Next();  // Consume ')'

            if (MatchType(TokenType.Colon))
            {
                returnTypeHint = GetTypeName();
            }
        }

        // Parse the function body
        List<ASTNode?> body = [];
        while (GetTokenName() != TokenName.KW_End)
        {
            var stmt = ParseStatement();
            if (stmt != null)
            {
                body.Add(stmt);
            }
        }

        Next();  // Consume 'end'

        PopNameStack();

        return new FunctionNode
        {
            Name = functionName,
            Parameters = parameters,
            Body = body,
            TypeHints = typeHints,
            ReturnTypeHint = returnTypeHint,
            IsOperatorOverload = isOperator
        };
    }

    private ForLoopNode? ParseForLoop()
    {
        MatchName(TokenName.KW_For);  // Consume 'for'

        var mangledNames = GetNameMap();
        var mangler = $"_{Guid.NewGuid().ToString()[..8]}_";
        HashSet<string> subMangled = [];
        var valueIteratorName = string.Empty;

        if (GetTokenType() == TokenType.Identifier)
        {
            valueIteratorName = mangler + token.Text;
            mangledNames[token.Text] = valueIteratorName;
            subMangled.Add(valueIteratorName);
        }

        var valueIterator = ParseIdentifier(false, false);
        ASTNode? indexIterator = null;

        if (MatchType(TokenType.Comma))
        {
            if (GetTokenType() == TokenType.Identifier)
            {
                mangledNames[token.Text] = mangler + token.Text;
                subMangled.Add(mangler + token.Text);
            }
            indexIterator = ParseIdentifier(false, false);
        }

        if (!MatchName(TokenName.KW_In))
        {
            throw new SyntaxError(GetErrorToken(), "Expected 'in' in for-loop.");
        }

        var dataSet = ParseExpression();

        if (!MatchName(TokenName.KW_Do))
        {
            throw new SyntaxError(GetErrorToken(), "Expected 'do' in for-loop.");
        }

        List<ASTNode?> body = [];
        while (GetTokenName() != TokenName.KW_End)
        {
            var stmt = ParseStatement();
            if (stmt != null)
            {
                body.Add(stmt);
            }
        }

        Next();  // Consume 'end'

        foreach (var mangledName in subMangled)
        {
            mangledNames.Remove(mangledName);
        }

        return new ForLoopNode(valueIterator ?? new IdentifierNode(valueIteratorName))
        {
            IndexIterator = indexIterator,
            DataSet = dataSet,
            Body = body
        };
    }

    private WhileLoopNode? ParseWhileLoop()
    {
        MatchName(TokenName.KW_While);  // Consume 'while'

        var condition = ParseExpression();

        if (!MatchName(TokenName.KW_Do))
        {
            throw new SyntaxError(GetErrorToken(), "Expected 'do' in while-loop.");
        }

        List<ASTNode?> body = [];
        while (GetTokenName() != TokenName.KW_End)
        {
            var stmt = ParseStatement();
            if (stmt != null)
            {
                body.Add(stmt);
            }
        }

        Next();  // Consume 'end'

        return new WhileLoopNode
        {
            Condition = condition,
            Body = body
        };
    }

    private RepeatLoopNode? ParseRepeatLoop()
    {
        MatchName(TokenName.KW_Repeat);  // Consume 'repeat'

        var count = ParseExpression();
        ASTNode? alias = null;

        if (MatchName(TokenName.KW_As))
        {
            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected identifier in repeat-loop value alias.");
            }

            alias = ParseIdentifier(false, false);
        }

        if (!MatchName(TokenName.KW_Do))
        {
            throw new SyntaxError(GetErrorToken(), "Expected 'do' in repeat-loop.");
        }

        List<ASTNode?> body = [];
        while (GetTokenName() != TokenName.KW_End)
        {
            var stmt = ParseStatement();
            if (stmt != null)
            {
                body.Add(stmt);
            }
        }

        Next();  // Consume 'end'

        return new RepeatLoopNode
        {
            Count = count,
            Alias = alias,
            Body = body
        };
    }

    private ReturnNode? ParseReturn()
    {
        MatchName(TokenName.KW_Return);
        var node = new ReturnNode();

        if (HasValue())
        {
            node.ReturnValue = ParseExpression();
        }

        if (MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            node.Condition = ParseExpression();
        }

        return node;
    }

    private ThrowNode? ParseThrow()
    {
        MatchName(TokenName.KW_Throw);
        var node = new ThrowNode();

        if (HasValue())
        {
            node.ErrorValue = ParseExpression();
        }

        if (MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            node.Condition = ParseExpression();
        }

        return node;
    }

    private ExitNode? ParseExit()
    {
        MatchName(TokenName.KW_Exit);
        var node = new ExitNode();

        if (HasValue())
        {
            node.ExitValue = ParseExpression();
        }

        if (MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            node.Condition = ParseExpression();
        }

        return node;
    }

    private BreakNode? ParseBreak()
    {
        MatchName(TokenName.KW_Break);
        var node = new BreakNode();

        if (MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            node.Condition = ParseExpression();
        }

        return node;
    }

    private NextNode? ParseNext()
    {
        MatchName(TokenName.KW_Next);
        var node = new NextNode();

        if (MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            node.Condition = ParseExpression();
        }

        return node;
    }

    private EvalNode? ParseEval()
    {
        MatchName(TokenName.KW_Eval);

        if (!HasValue())
        {
            throw new SyntaxError(GetErrorToken(), "Expected value after 'eval'.");
        }

        return new EvalNode(ParseExpression());
    }

    private ExportNode? ParseExport()
    {
        MatchName(TokenName.KW_Export);

        if (!HasValue())
        {
            throw new SyntaxError(GetErrorToken(), "Expected value after 'export'.");
        }

        return new ExportNode(ParseExpression());
    }

    private ImportNode? ParseImport()
    {
        MatchName(TokenName.KW_Import);

        if (!HasValue())
        {
            throw new SyntaxError(GetErrorToken(), "Expected value after 'import'.");
        }

        return new ImportNode(ParseExpression());
    }

    private IncludeNode? ParseInclude()
    {
        MatchName(TokenName.KW_Include);

        if (!HasValue())
        {
            throw new SyntaxError(GetErrorToken(), "Expected path after 'include'.");
        }

        var package = ParseExpression() ?? throw new SyntaxError(GetErrorToken(), "Expected path after 'include`.");
        return new IncludeNode(package);
    }

    private PackageNode? ParsePackage()
    {
        MatchName(TokenName.KW_Package);

        if (GetTokenType() is not TokenType.Identifier and
            not TokenType.Typename)
        {
            throw new SyntaxError(GetErrorToken(), "Expected identifier for package name.");
        }

        var packageName = ParseIdentifier(false, true);

        List<ASTNode?> body = [];
        while (GetTokenName() != TokenName.KW_End)
        {
            var stmt = ParseStatement();
            if (stmt != null)
            {
                body.Add(stmt);
            }
        }

        Next();  // Consume 'end'

        return new PackageNode(packageName)
        {
            Body = body
        };
    }

    private CaseNode? ParseCase()
    {
        if (!MatchName(TokenName.KW_Case))
        {
            throw new SyntaxError(GetErrorToken(), "Expected case-statement.");
        }

        var node = new CaseNode();
        var mangledNames = GetNameMap();
        HashSet<string> subMangled = [];

        if (HasValue())
        {
            node.TestValue = ParseExpression();

            // case {condition} as {identifier}
            if (MatchName(TokenName.KW_As))
            {
                if (GetTokenType() != TokenType.Identifier)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected an identifier.");
                }

                var mangler = $"_{Guid.NewGuid().ToString()[..8]}_";
                var testAlias = mangler + token.Text;
                mangledNames[token.Text] = testAlias;
                subMangled.Add(testAlias);

                node.TestValueAlias = ParseIdentifier(false, false);
            }
        }

        while (GetTokenName() != TokenName.KW_End)
        {
            if (MatchName(TokenName.KW_When))
            {
                var caseWhen = new CaseWhenNode();
                if (!HasValue())
                {
                    throw new SyntaxError(GetErrorToken(), "Expected condition or value for case-when.");
                }

                caseWhen.Condition = ParseExpression();

                while (GetTokenName() is not TokenName.KW_When and not TokenName.KW_Else and
                       not TokenName.KW_End)
                {
                    var stmt = ParseStatement();
                    if (stmt != null)
                    {
                        caseWhen.Body.Add(stmt);
                    }
                }

                node.WhenNodes.Add(caseWhen);
            }
            else if (MatchName(TokenName.KW_Else))
            {
                while (GetTokenName() != TokenName.KW_End)
                {
                    var stmt = ParseStatement();
                    if (stmt != null)
                    {
                        node.ElseBody.Add(stmt);
                    }
                }
            }
        }

        Next();  // Consume 'end'

        foreach (var name in subMangled)
        {
            mangledNames.Remove(name);
        }

        return node;
    }

    private IfNode? ParseIf()
    {
        if (!MatchName(TokenName.KW_If))
        {
            throw new SyntaxError(GetErrorToken(), "Expected if-statement.");
        }

        if (!HasValue())
        {
            throw new SyntaxError(GetErrorToken(), "Expected condition after 'if'.");
        }

        var node = new IfNode
        {
            Condition = ParseExpression()
        };

        int blocks = 1;
        var building = TokenName.KW_If;

        while (stream.CanRead && blocks > 0)
        {
            var subType = GetTokenName();
            if (subType == TokenName.KW_End && blocks >= 1)
            {
                --blocks;

                // Stop here.
                if (blocks == 0)
                {
                    Next();
                    break;
                }
            }
            else if (blocks == 1 && subType == TokenName.KW_Else)
            {
                if (building != TokenName.KW_Else)
                {
                    Next();
                    building = TokenName.KW_Else;
                }
            }
            else if (blocks == 1 && subType == TokenName.KW_ElseIf)
            {
                Next();
                building = TokenName.KW_ElseIf;

                var elsif = new IfNode();

                if (!HasValue())
                {
                    throw new SyntaxError(GetErrorToken(), "Expected condition after 'elsif'.");
                }

                elsif.Condition = ParseExpression();
                node.ElsifNodes?.Add(elsif);
            }

            var stmt = ParseStatement();
            if (building == TokenName.KW_If && stmt != null)
            {
                node.Body.Add(stmt);
            }
            else if (building == TokenName.KW_ElseIf && stmt != null)
            {
                node.ElsifNodes?.Last()?.Body.Add(stmt);
            }
            else if (building == TokenName.KW_Else && stmt != null)
            {
                node.ElseBody.Add(stmt);
            }
        }

        return node;
    }

    private TryNode? ParseTry()
    {
        MatchName(TokenName.KW_Try);  // Consume 'try'

        List<ASTNode?> tryBody = [];
        List<ASTNode?> catchBody = [];
        List<ASTNode?> finallyBody = [];
        ASTNode? errorType = null;
        ASTNode? errorMessage = null;

        int blocks = 1;
        var building = TokenName.KW_Try;

        while (stream.CanRead && blocks > 0)
        {
            var subType = GetTokenName();

            if (subType == TokenName.KW_End && blocks >= 1)
            {
                --blocks;

                // Stop here.
                if (blocks == 0)
                {
                    Next();
                    break;
                }
            }
            else if (blocks == 1 && subType == TokenName.KW_Catch)
            {
                if (building != TokenName.KW_Catch)
                {
                    Next();  // Consume 'catch'
                    if (MatchType(TokenType.LParen))
                    {
                        if (GetTokenType() != TokenType.Identifier)
                        {
                            throw new SyntaxError(GetErrorToken(), "Expected identifier in catch parameters.");
                        }
                        var firstParameter = ParseIdentifier(false, false);
                        if (MatchType(TokenType.Comma))
                        {
                            if (GetTokenType() != TokenType.Identifier)
                            {
                                throw new SyntaxError(GetErrorToken(), "Expected identifier in catch parameters.");
                            }

                            errorType = firstParameter;
                            errorMessage = ParseIdentifier(false, false);
                        }
                        else
                        {
                            errorMessage = firstParameter;
                        }

                        if (!MatchType(TokenType.RParen))
                        {
                            throw new SyntaxError(GetErrorToken(), "Expected ')' in catch parameter expression.");
                        }
                    }
                    building = TokenName.KW_Catch;
                    continue;
                }
            }
            else if (blocks == 1 && subType == TokenName.KW_Finally)
            {
                if (building != TokenName.KW_Finally)
                {
                    Next();  // Consume 'finally'
                    building = TokenName.KW_Finally;
                    continue;
                }
            }

            var stmt = ParseStatement();
            if (building == TokenName.KW_Try && stmt != null)
            {
                tryBody.Add(stmt);
            }
            else if (building == TokenName.KW_Catch && stmt != null)
            {
                catchBody.Add(stmt);
            }
            else if (building == TokenName.KW_Finally && stmt != null)
            {
                finallyBody.Add(stmt);
            }
        }

        return new TryNode
        {
            TryBody = tryBody,
            CatchBody = catchBody,
            FinallyBody = finallyBody,
            ErrorMessage = errorMessage,
            ErrorType = errorType
        };
    }

    private FunctionCallNode? ParseFunctionCall(string identifierName, TokenName type)
    {
        List<ASTNode?> arguments = CollectCallArguments();
        return new FunctionCallNode(identifierName, type, arguments);
    }

    private LambdaCallNode? ParseLambdaCall(ASTNode? lambdaNode)
    {
        List<ASTNode?> arguments = CollectCallArguments();
        return new LambdaCallNode(lambdaNode, arguments);
    }

    private List<ASTNode?> CollectCallArguments()
    {
        Next();  // Consume the '('

        List<ASTNode?> arguments = [];
        while (GetTokenType() != TokenType.RParen)
        {
            arguments.Add(ParseExpression());

            if (GetTokenType() == TokenType.Comma)
            {
                Next();
            }
            else if (GetTokenType() != TokenType.RParen)
            {
                throw new SyntaxError(GetErrorToken(), "Expected ')' or ',' in lambda call.");
            }
        }

        Next();
        return arguments;
    }

    private LambdaNode? ParseLambda()
    {
        MatchType(TokenType.Lambda);  // Consume 'with'

        var mangler = $"_{Guid.NewGuid().ToString()[..8]}_";
        Dictionary<string, string> localNames = [];
        Dictionary<string, int> typeHints = [];
        var returnTypeHint = TypeRegistry.GetType("any");
        var mangledNames = PushNameStack();

        // Parse parameters
        List<KeyValuePair<string, ASTNode?>> parameters = [];
        if (GetTokenType() == TokenType.LParen)
        {
            Next();  // Consume '('

            while (GetTokenType() != TokenType.RParen)
            {
                if (GetTokenType() != TokenType.Identifier)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected parameter name.");
                }

                var paramName = token.Text;

                if (mangledNames.ContainsKey(paramName))
                {
                    throw new SyntaxError(GetErrorToken(), $"The parameter name '{paramName}' is already used.");
                }

                var mangledName = mangler + paramName;
                mangledNames[paramName] = mangledName;
                ASTNode? defaultValue = null;
                Next();

                // Check for type hint.
                if (MatchType(TokenType.Colon))
                {
                    var typeName = GetTypeName();
                    typeHints[mangledName] = typeName;
                }

                // Check for default value
                if (GetTokenType() == TokenType.Operator &&
                    GetTokenName() == TokenName.Ops_Assign)
                {
                    Next();  // Consume '='
                    defaultValue = ParseExpression();
                }

                parameters.Add(new KeyValuePair<string, ASTNode?>(mangledName, defaultValue));

                if (GetTokenType() == TokenType.Comma)
                {
                    Next();
                }
                else if (GetTokenType() != TokenType.RParen)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected ',' or ')' in parameter list.");
                }
            }

            Next();  // Consume ')'

            if (MatchType(TokenType.Colon))
            {
                returnTypeHint = GetTypeName();
            }
        }

        if (!MatchName(TokenName.KW_Do))
        {
            throw new SyntaxError(GetErrorToken(), "Expected 'do' in lambda expression.");
        }

        // Parse the lambda body
        List<ASTNode?> body = [];
        while (GetTokenName() != TokenName.KW_End)
        {
            var stmt = ParseStatement();
            if (stmt != null)
            {
                body.Add(stmt);
            }
        }

        Next();  // Consume 'end'

        PopNameStack();

        return new LambdaNode
        {
            Parameters = parameters,
            Body = body,
            TypeHints = typeHints,
            ReturnTypeHint = returnTypeHint
        };
    }

    private PrintNode? ParsePrint()
    {
        var printNode = new PrintNode();
        var name = GetTokenName();

        // Consume 'print', 'println', 'eprint', 'eprintln'
        MatchType(TokenType.Keyword);

        var printNewLine = name is TokenName.KW_PrintLn or TokenName.KW_EPrintLn;
        var printStdError = name is TokenName.KW_EPrint or TokenName.KW_EPrintLn;
        var expression = ParseExpression();
        return new PrintNode(expression, printNewLine, printStdError);
    }

    private PrintXyNode? ParsePrintXy()
    {
        var printNode = new PrintXyNode();
        MatchType(TokenType.Keyword);

        if (!MatchType(TokenType.LParen))
        {
            throw new SyntaxError(GetErrorToken(), "Expected a value between '(' and ')'.");
        }

        while (stream.CanRead && GetTokenType() != TokenType.RParen)
        {
            if (GetTokenType() == TokenType.Comma)
            {
                Next();
                continue;
            }

            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected an expression.");
            }

            if (printNode.Expression == null)
            {
                printNode.Expression = ParseExpression();
                continue;
            }

            if (printNode.X == null)
            {
                printNode.X = ParseExpression();
                continue;
            }

            if (printNode.Y == null)
            {
                printNode.Y = ParseExpression();
                continue;
            }

            if (printNode.Expression != null && printNode.X != null && printNode.Y != null)
            {
                throw new SyntaxError(GetErrorToken(), "Wrong number of parameters for printxy.");
            }
        }

        Next();  // Consume ")"

        if (printNode.Expression == null || printNode.X == null || printNode.Y == null)
        {
            throw new SyntaxError(GetErrorToken(), "Wrong number of parameters for printxy.");
        }

        return printNode;
    }

    private ASTNode? ParseLiteral()
    {
        if (GetTokenType() == TokenType.Typename && Peek().Type == TokenType.Qualifier)
        {
            return ParseIdentifier(false, false);
        }

        LiteralNode literalNode = new(Value.Create(token.Value));
        Next();  // Consume literal
        return literalNode;
    }

    private HashLiteralNode? ParseHashLiteral()
    {
        Dictionary<ASTNode, ASTNode?> elements = [];
        List<string> keys = [];

        MatchType(TokenType.LBrace);  // Consume '{'

        while (GetTokenType() != TokenType.RBrace)
        {
            if (GetTokenType() is not TokenType.String and
                not TokenType.Identifier and
                not TokenType.Literal)
            {
                throw new SyntaxError(GetErrorToken(), "Expected a string, literal, or identifier for hashmap key.");
            }

            var keyString = token.Text;
            var key = ParseExpression() ?? throw new SyntaxError(GetErrorToken(), "Expected a key for hashmap.");

            if (key.Type == ASTNodeType.Identifier)
            {
                key = new LiteralNode(Value.Create(keyString));
            }

            if (!MatchType(TokenType.Colon))
            {
                throw new SyntaxError(GetErrorToken(), "Expected ':' in hashmap literal");
            }

            keys.Add(keyString);

            var value = ParseExpression();
            elements.Add(key, value);

            if (GetTokenType() == TokenType.Comma)
            {
                Next();  // Consume ','
            }
            else if (GetTokenType() != TokenType.RBrace)
            {
                throw new SyntaxError(GetErrorToken(), "Expected '}' or ',' in hashmap literal");
            }
        }

        MatchType(TokenType.RBrace);  // Consume '}'

        return new HashLiteralNode(elements, keys);
    }

    private ASTNode? ParseListLiteral()
    {
        List<ASTNode> elements = [];

        MatchType(TokenType.LBracket);  // Consume '['
        var isRange = false;

        while (GetTokenType() != TokenType.RBracket)
        {
            elements.Add(ParseExpression() ?? throw new SyntaxError(GetErrorToken(), "Expected an expression in list literal."));

            if (!isRange && GetTokenType() == TokenType.Comma)
            {
                Next();  // Consume ','
            }
            else if (!isRange && (GetTokenType() == TokenType.Range || GetTokenName() == TokenName.KW_To))
            {
                isRange = true;
                Next();  // Consume '..'
            }
            else if (GetTokenType() != TokenType.RBracket)
            {
                if (!isRange)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected ']' or ',' in list literal.");
                }
                else
                {
                    throw new SyntaxError(GetErrorToken(), "Expected ']' or '..' in range literal.");
                }
            }
        }

        if (!MatchType(TokenType.RBracket))
        {
            throw new SyntaxError(GetErrorToken(), "Expected ']' in list or range literal.");
        }

        if (isRange)
        {
            if (elements.Count != 2)
            {
                throw new SyntaxError(GetErrorToken(), "Expected start and end values in range literal.");
            }

            return new RangeLiteralNode(elements[0], elements[1]);
        }

        return new ListLiteralNode(elements);
    }

    private ASTNode? ParseIndexing(ASTNode? indexedObject)
    {
        return ParseIndexingInternal(indexedObject);
    }

    private ASTNode? ParseIndexing(string identifierName)
    {
        ASTNode? baseNode = new IdentifierNode(identifierName);
        return ParseIndexingInternal(baseNode);
    }

    private ASTNode? ParseIndexingInternal(ASTNode? baseNode)
    {
        if (!MatchType(TokenType.LBracket))
        {
            return baseNode;
        }

        if (MatchType(TokenType.RBracket))
        {
            return baseNode;
        }

        var isSlice = false;
        Token indexValueToken = GetErrorToken(false);

        ASTNode? start = null;
        ASTNode? stop = null;
        ASTNode? step = null;

        if (GetTokenType() is not TokenType.Colon and
            not TokenType.Qualifier)
        {
            start = ParseExpression();
        }

        if (MatchType(TokenType.Colon) || GetTokenType() == TokenType.Qualifier)
        {
            isSlice = true;

            if (GetTokenType() is not TokenType.Colon and
                not TokenType.Qualifier and
                not TokenType.RBracket)
            {
                stop = ParseExpression();
            }

            if (MatchType(TokenType.Colon) || MatchType(TokenType.Qualifier))
            {
                if (GetTokenType() != TokenType.RBracket)
                {
                    step = ParseExpression();
                }
            }
        }

        MatchType(TokenType.RBracket);
        ASTNode? node;

        if (isSlice)
        {
            node = new SliceNode(baseNode, start, stop, step);
        }
        else
        {
            var indexExpression = start ?? throw new SyntaxError(GetErrorToken(), "Missing index expression.");
            if (indexExpression.Type is not ASTNodeType.Literal and
                not ASTNodeType.Identifier and
                not ASTNodeType.FunctionCall and
                not ASTNodeType.BinaryOperation and
                not ASTNodeType.MethodCall and
                not ASTNodeType.MemberAccess)
            {
                throw new SyntaxError(indexValueToken, "Invalid index value in indexer.");
            }

            node = new IndexingNode(baseNode, indexExpression);
        }

        if (IsAssignmentOperator())
        {
            var op = GetTokenName();
            Next();
            var initializer = ParseExpression();
            node = new IndexAssignmentNode(node, op,
                                                         initializer);
        }

        return node;
    }

    private ASTNode? ParseMemberAccess(ASTNode? left)
    {
        while (GetTokenType() == TokenType.Dot)
        {
            Next();  // Consume '.'

            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected identifier after '.' in member access.");
            }

            var op = GetTokenName();
            var memberName = token.Text;
            Next();

            if (GetTokenType() == TokenType.LParen)
            {
                left = ParseMethodCall(left, memberName, op);
            }
            else if (IsAssignmentOperator())
            {
                left = ParseMemberAssignment(left, memberName);
            }
            else
            {
                left = new MemberAccessNode(left, memberName);
            }
        }

        return left;
    }

    private MethodCallNode? ParseMethodCall(
        ASTNode? obj, string methodName,
        TokenName type)
    {
        Next();  // Consume '('

        // Parse function arguments
        List<ASTNode?> arguments = [];
        while (GetTokenType() != TokenType.RParen)
        {
            arguments.Add(ParseExpression());

            if (GetTokenType() == TokenType.Comma)
            {
                Next();  // Consume ','
            }
            else if (GetTokenType() != TokenType.RParen)
            {
                throw new SyntaxError(GetErrorToken(), "Expected ')' or ',' in function call.");
            }
        }

        Next();  // Consume the closing parenthesis ')'

        return new MethodCallNode(obj, methodName, type,
                                                arguments);
    }

    private MemberAssignmentNode? ParseMemberAssignment(
        ASTNode? obj, string memberName)
    {
        var type = GetTokenName();
        Next();

        var initializer = ParseExpression();
        return new MemberAssignmentNode(obj, memberName, type, initializer);
    }

    private PackAssignmentNode? ParsePackAssignment(ASTNode? baseNode)
    {
        /*
        a, b, c =< 0, 1, 2             # a = 0, b = 1, c = 2
        a, b =< get_zero_and_one()     # a = 0, b = 1
        */
        var assignment = new PackAssignmentNode();
        assignment.Left.Add(baseNode);

        while (stream.CanRead && GetTokenType() == TokenType.Comma)
        {
            MatchType(TokenType.Comma);
            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected identifier in pack assignment variable set.");
            }

            var identifierName = token.Text;
            if (HasName(identifierName))
            {
                identifierName = GetName(identifierName);
            }
            Next();

            assignment.Left.Add(
                new IdentifierNode(identifierName));
        }

        if (!MatchName(TokenName.Ops_Unpack))
        {
            throw new SyntaxError(GetErrorToken(), "Expected an unpack operator, '=<', in pack assignment.");
        }

        var lhsLength = assignment.Left.Count;

        while (stream.CanRead && HasValue())
        {
            // we have everything we need.
            if (assignment.Right.Count == lhsLength)
            {
                break;
            }

            var rhs = ParseExpression();
            assignment.Right.Add(rhs);

            if (GetTokenType() == TokenType.Comma)
            {
                Next();
            }
            else
            {
                // we're at the end of the statement.
                break;
            }
        }

        return assignment;
    }

    private ConstAssignmentNode? ParseConstAssignment()
    {
        MatchName(TokenName.KW_Const);  // skip the const

        if (GetTokenType() != TokenType.Identifier)
        {
            throw new SyntaxError(GetErrorToken(), "Expected an identifier in const assignment.");
        }

        var identifierName = token.Text;
        if (identifierName != identifierName.ToUpper())
        {
            throw new SyntaxError(GetErrorToken(), "Constant identifiers should contain only uppercase characters and underscores.");
        }

        Next();  // skip the identifier
        var type = GetTokenName();

        if (!IsAssignmentOperator(type))
        {
            throw new SyntaxError(GetErrorToken(), "Expected an assignment operator in constant assignment.");
        }

        if (type != TokenName.Ops_Assign)
        {
            throw new SyntaxError(GetErrorToken(), "Expected an assignment operator in constant assignment.");
        }
        Next();  // skip the operator

        var initializer = ParseExpression();

        return new ConstAssignmentNode(identifierName, type,
                                                     initializer);
    }

    private AssignmentNode? ParseAssignment(ASTNode? baseNode, string identifierName)
    {
        if (!IsAssignmentOperator())
        {
            throw new SyntaxError(GetErrorToken(), "Expected an assignment operator in assignment.");
        }

        var type = GetTokenName();
        AssignmentNode? assignment = null;

        if (type == TokenName.KW_Case)
        {
            assignment = new(baseNode, identifierName, type, ParseCase());
        }
        else
        {
            Next();

            assignment = new(baseNode, identifierName, type, ParseExpression());
        }

        if (assignment != null && MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            assignment.Condition = ParseExpression();
        }

        return assignment;
    }

    private ASTNode? ParseQualifiedIdentifier(string prefix)
    {
        if (!MatchType(TokenType.Qualifier))
        {
            throw new SyntaxError(GetErrorToken(), "Expected a qualifier.");
        }

        var isTypeName = GetTokenType() == TokenType.Typename;

        if (GetTokenType() != TokenType.Identifier && !isTypeName)
        {
            throw new SyntaxError(GetErrorToken(), "Expected an identifier after qualifier.");
        }

        var rightIdentifierName = token.Text;
        Next();

        var qualifiedName = $"{prefix}::{rightIdentifierName}";

        ASTNode? qualifiedNode =
            new IdentifierNode(qualifiedName);

        if (GetTokenType() == TokenType.Dot)
        {
            qualifiedNode = ParseMemberAccess(qualifiedNode);
        }
        else if (GetTokenType() == TokenType.LParen)
        {
            qualifiedNode = ParseFunctionCall(qualifiedName, GetTokenName());
        }
        else if (GetTokenType() == TokenType.LBracket)
        {
            qualifiedNode = ParseIndexing(qualifiedName);
        }
        else if (GetTokenType() == TokenType.Qualifier)
        {
            qualifiedNode = ParseQualifiedIdentifier(qualifiedName);
        }
        else if (isTypeName)
        {
            throw new SyntaxError(GetErrorToken(), "Expected '(' or '::' after the identifier `" + qualifiedName + "`.");
        }

        return qualifiedNode;
    }

    private ASTNode? ParseIdentifier(bool packed, bool lenient)
    {
        var idToken = token.Clone();
        bool isInstance = MatchName(TokenName.KW_This);

        var isTypeName = GetTokenType() == TokenType.Typename;

        if (GetTokenType() != TokenType.Identifier && !isTypeName)
        {
            if (isInstance)
            {
                return new SelfNode();
            }

            throw new SyntaxError(GetErrorToken(), "Expected an identifier.");
        }

        var type = GetTokenName();
        var identifierName = (isInstance ? "@" : string.Empty) + token.Text;

        if (HasName(identifierName))
        {
            identifierName = GetName(identifierName);
        }

        Next();

        ASTNode? node;

        if (isInstance)
        {
            node = new SelfNode(identifierName);
        }
        else
        {
            node = new IdentifierNode(identifierName);
        }

        switch (GetTokenType())
        {
            case TokenType.Dot:
                node = ParseMemberAccess(node);
                break;
            case TokenType.LParen:
                node = ParseFunctionCall(identifierName, type);
                break;
            case TokenType.LBracket:
                node = ParseIndexing(identifierName);
                break;
            case TokenType.Operator when IsAssignmentOperator():
                node = ParseAssignment(node, identifierName);
                break;
            case TokenType.Qualifier when Peek().Type == TokenType.Identifier:
                node = ParseQualifiedIdentifier(identifierName);
                break;
            case TokenType.Comma when !packed && LookAhead([TokenName.Ops_Unpack]):
                node = ParsePackAssignment(node);
                break;
            default:
                if (isTypeName && !lenient)
                {
                    throw new SyntaxError(GetErrorToken(), $"Expected '(' or '::' after the identifier `{identifierName}`.");
                }
                else
                {
                    node = new IdentifierNode(identifierName);
                }

                break;
        }

        if (node != null)
        {
            node.Token = idToken;
        }

        return node;
    }

    private ASTNode? ParseExpression()
    {
        var node = ParseLogicalOr();

        if (GetTokenType() == TokenType.Question)
        {
            Next();  // Consume '?'

            var trueBranch = ParseExpression();

            if (!MatchType(TokenType.Colon))
            {
                throw new SyntaxError(GetErrorToken(), "Expected ':' in ternary operation.");
            }

            var falseBranch = ParseExpression();  // Parse the false branch

            return new TernaryOperationNode(node, trueBranch, falseBranch);
        }

        if (node?.Type == ASTNodeType.Lambda && GetTokenType() == TokenType.LParen)
        {
            node = ParseLambdaCall(node);
        }

        return node;
    }

    private ASTNode? ParseLogicalOr()
    {
        var left = ParseLogicalAnd();

        while (stream.CanRead && (GetTokenName() == TokenName.Ops_Or || GetTokenName() == TokenName.Ops_NullCoalesce))
        {
            var op = GetTokenName();
            Next();  // Consume '||' or '??'

            var right = ParseLogicalAnd();
            left = new BinaryOperationNode(left, op, right);
        }

        return left;
    }

    private ASTNode? ParseLogicalAnd()
    {
        var left = ParseBitwiseOr();

        while (stream.CanRead && GetTokenName() == TokenName.Ops_And)
        {
            Next();  // Consume '&&'

            var right = ParseBitwiseOr();
            left = new BinaryOperationNode(left, TokenName.Ops_And, right);
        }

        return left;
    }

    private ASTNode? ParseBitwiseOr()
    {
        var left = ParseBitwiseXor();

        while (stream.CanRead && GetTokenName() == TokenName.Ops_BitwiseOr)
        {
            Next();  // Consume '|'

            var right = ParseBitwiseXor();
            left = new BinaryOperationNode(left, TokenName.Ops_BitwiseOr, right);
        }

        return left;
    }

    private ASTNode? ParseBitwiseXor()
    {
        var left = ParseBitwiseAnd();

        while (stream.CanRead && GetTokenName() == TokenName.Ops_BitwiseXor)
        {
            Next();  // Consume '^'

            var right = ParseBitwiseAnd();
            left = new BinaryOperationNode(left, TokenName.Ops_BitwiseXor, right);
        }

        return left;
    }

    private ASTNode? ParseBitwiseAnd()
    {
        var left = ParseEquality();

        while (stream.CanRead && GetTokenName() == TokenName.Ops_BitwiseAnd)
        {
            Next();  // Consume '&'

            var right = ParseEquality();
            left = new BinaryOperationNode(left, TokenName.Ops_BitwiseAnd, right);
        }

        return left;
    }

    private ASTNode? ParseEquality()
    {
        var left = ParseComparison();

        while (stream.CanRead && IsEqualityOperator())
        {
            var op = GetTokenName();

            Next();  // Skip operator

            var right = ParseComparison();
            left = new BinaryOperationNode(left, op, right);
        }

        return left;
    }

    private ASTNode? ParseComparison()
    {
        var left = ParseBitshift();

        while (stream.CanRead && IsComparisonOperator())
        {
            var op = GetTokenName();

            Next();  // Skip operator

            var right = ParseBitshift();
            left = new BinaryOperationNode(left, op, right);
        }

        return left;
    }

    private ASTNode? ParseBitshift()
    {
        var left = ParseAdditive();

        while (stream.CanRead && IsBitwiseOperator())
        {
            var op = GetTokenName();

            Next();  // Skip operator

            var right = ParseAdditive();
            left = new BinaryOperationNode(left, op, right);
        }

        return left;
    }

    private ASTNode? ParseAdditive()
    {
        var left = ParseMultiplicative();
        while (stream.CanRead && IsAdditiveOperator())
        {
            var op = GetTokenName();
            Next();  // Skip operator
            var right = ParseMultiplicative();
            left = new BinaryOperationNode(left, op, right);
        }
        return left;
    }

    private ASTNode? ParseMultiplicative()
    {
        var left = ParseUnary();

        while (stream.CanRead && IsMultiplicativeOperator())
        {
            var op = GetTokenName();

            Next();  // Skip operator

            var right = ParseUnary();
            left = new BinaryOperationNode(left, op, right);
        }

        return left;
    }

    private ASTNode? ParseUnary()
    {
        while (stream.CanRead && IsUnaryOperator())
        {
            var op = GetTokenName();

            Next();  // Skip operator

            var right = ParseUnary();
            return new UnaryOperationNode(op, right);
        }

        return ParsePrimary();
    }

    private ASTNode? ParsePrimary()
    {
        ASTNode? node;
        var nodeToken = token.Clone();

        switch (GetTokenType())
        {
            case TokenType.Identifier:
                node = ParseIdentifier(false, false);
                break;

            case TokenType.Error:
                node = ParseError();
                break;

            case TokenType.Keyword:
                node = ParseKeyword();
                break;

            case TokenType.Literal:
            case TokenType.String:
            case TokenType.Typename:
            case TokenType.Bytes:
                node = ParseLiteral();
                break;

            case TokenType.LParen:
                {
                    Next();  // Skip "("
                    if (GetTokenType() == TokenType.RParen)
                    {
                        throw new SyntaxError(GetErrorToken(), "Expected a value between '(' and ')'.");
                    }
                    var result = ParseExpression();
                    MatchType(TokenType.RParen);
                    node = result;
                }
                break;

            case TokenType.LBracket:
                node = ParseListLiteral();
                break;

            case TokenType.LBrace:
                node = ParseHashLiteral();
                break;

            default:
                if (GetTokenName() == TokenName.KW_Lambda)
                {
                    node = ParseLambda();
                }
                else if (GetTokenName() == TokenName.KW_Case)
                {
                    node = ParseCase();
                }
                else
                {
                    throw new SyntaxError(GetErrorToken(), $"Unexpected token `{token.Text}`.");
                }
                break;
        }

        switch (GetTokenType())
        {
            case TokenType.Dot:
                node = ParseMemberAccess(node);
                break;
            case TokenType.LBracket:
                node = ParseIndexing(node);
                break;
        }

        if (node != null)
        {
            node.Token = nodeToken;
        }

        return node;
    }
}