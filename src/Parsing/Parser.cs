using kiwi.Parsing.AST;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Parsing;

public partial class Parser
{
    private ASTNode? ParseKeyword()
    {
        switch (GetTokenName())
        {
            case TokenName.KW_Require:
                return ParseRequire();

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
            case TokenName.KW_StaticSelf:
                return ParseIdentifier(false, false);

            case TokenName.KW_Repeat:
                return ParseRepeatLoop();

            case TokenName.KW_Try:
                return ParseTry();

            case TokenName.KW_Return:
                return ParseReturn();

            case TokenName.KW_Yield:
                return ParseYield();

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

            case TokenName.KW_Abstract:
                Next(); // consume 'abstract'
                if (GetTokenName() != TokenName.KW_Struct)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected 'struct' after 'abstract'.");
                }
                return ParseStruct(isAbstract: true);

            case TokenName.KW_Enum:
                return ParseEnum();

            case TokenName.KW_Struct:
                return ParseStruct();

            case TokenName.KW_Interface:
                return ParseInterface();

            case TokenName.KW_Method:
                return ParseFunction();

            case TokenName.KW_Var:
                return ParseVar();

            case TokenName.KW_Super:
                return ParseSuperCall();

            default:
                throw new SyntaxError(GetErrorToken(), $"Unexpected keyword '{token.Text}'.");
        }
    }

    private ASTNode? ParseStatement(bool applyWhenGuard = true)
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

            // when super.method() is used as a statement, route through ParseExpression so
            // operators and chained calls after the super call are parsed correctly.
            case TokenType.Keyword when GetTokenName() == TokenName.KW_Super:
                node = ParseExpression();
                break;

            // when @var or @@var used as a statement, route through ParseExpression so binary
            // operators (>=, ==, &&, etc.) are parsed correctly after the member access.
            case TokenType.Keyword when GetTokenName() is TokenName.KW_This or TokenName.KW_StaticSelf:
                if (GetTokenName() == TokenName.KW_This && IsDecoratorPattern())
                    node = ParseDecoratedFunction();
                else
                    node = ParseExpression();
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
            case TokenType.Interpolation:
                node = ParseExpression();
                break;

            case TokenType.Newline:
                return null;

            default:
                if (nodeToken.Type == TokenType.Eof)
                {
                    throw new UnexpectedEndOfFileError(GetErrorToken());
                }

                throw new TokenStreamError(GetErrorToken(), $"Unexpected token in statement: {Enum.GetName(nodeToken.Type)}: `{nodeToken.Text}`");
        }

        // Apply a when-guard to any statement that did not already consume 'when'.
        // return/throw/exit/break/next/assignment all consume 'when' themselves,
        // so MatchName returns false for those and no wrapping occurs.
        if (applyWhenGuard && node != null && MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            var whenCondition = ParseExpression();
            var ifNode = new IfNode { Condition = whenCondition, Body = [node], Token = nodeToken };

            if (MatchName(TokenName.KW_Else))
            {
                if (GetTokenType() == TokenType.Eof)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected statement after 'else'.");
                }

                var elseStmt = ParseStatement(applyWhenGuard: false);
                if (elseStmt != null)
                {
                    ifNode.ElseBody.Add(elseStmt);
                }
            }

            node = ifNode;
        }

        if (node != null)
        {
            node.Token = nodeToken;
            return node;
        }

        return node;
    }

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
        // Descriptive error messages (e.g. from invalid byte literals) contain spaces
        // and are intended to be catchable at runtime via try/catch.
        // Raw unrecognized tokens (e.g. "//") have no spaces and are parse-time errors.
        if (token.Text.Contains(' '))
        {
            var errorValue = new LiteralNode(Value.CreateString(token.Text));
            Next();
            return new ThrowNode(errorValue, null);
        }

        throw new SyntaxError(GetErrorToken(), $"Unexpected token '{token.Text}'.");
    }

    private RequireNode? ParseRequire()
    {
        MatchName(TokenName.KW_Require);

        if (!HasValue())
        {
            throw new SyntaxError(GetErrorToken(), "Expected a package name.");
        }

        var packageName = GetTokenText();
        Next();

        return new RequireNode(packageName, PackagesDefined.Contains(packageName));
    }

    /// <summary>
    /// Parses an event name without chaining postfix operators, so that a following '('
    /// is not greedily consumed as a call on the event-name expression.
    /// Accepts string literals, interpolated strings, and bare identifiers (variables).
    /// </summary>
    private ASTNode ParseEventName(string context)
    {
        switch (GetTokenType())
        {
            case TokenType.String:
            {
                var lit = ParseLiteral();
                return lit ?? throw new SyntaxError(GetErrorToken(), $"Expected an event name in '{context}'.");
            }
            case TokenType.Interpolation:
            {
                var interp = ParseInterpolation();
                return interp ?? throw new SyntaxError(GetErrorToken(), $"Expected an event name in '{context}'.");
            }
            case TokenType.Identifier:
            {
                var idToken = token.Clone();
                var name = GetTokenText();
                Next();
                return new IdentifierNode(name) { Token = idToken };
            }
            default:
                throw new SyntaxError(GetErrorToken(), $"Expected a string-literal for event name in '{context}'.");
        }
    }

    private OffNode ParseOff()
    {
        MatchName(TokenName.KW_Off);

        var eventName = ParseEventName("off");
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

        var eventName = ParseEventName("emit");

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
        ASTNode eventName = ParseEventName("on/once");
        ASTNode? callback = null;

        // Optional priority: on "event", 10 do ... end
        int priority = 0;
        if (GetTokenType() == TokenType.Comma)
        {
            Next(); // consume ','
            if (GetTokenType() != TokenType.Literal || !token.Value.IsInteger())
            {
                throw new SyntaxError(GetErrorToken(), "Expected an integer priority after ','.");
            }
            priority = (int)token.Value.GetInteger();
            Next(); // consume integer
        }

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
            var openToken = token;
            Next(); // Consume 'do'

            var body = ParseSimpleBlock(openToken);

            callback = new LambdaNode
            {
                Parameters = [],
                Body = body,
                TypeHints = [],
                ReturnTypeHint = [TypeRegistry.GetType("any")],
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
            return new OnNode(eventName, callback, priority);
        }
        else if (eventType == TokenName.KW_Once)
        {
            return new OnceNode(eventName, callback, priority);
        }

        throw new SyntaxError(GetErrorToken(), "Expected an event handler.");
    }

    private ASTNode? ParseDo()
    {
        var openToken = token;
        MatchName(TokenName.KW_Do);  // Consume 'do'

        /*
        do 
          [ statements ]
        [ when when_condition ]
        end

        do (params)
          [ statements ]
        end

        do (params) -> [ statement ]
        */

        if (GetTokenType() == TokenType.LParen)
        {
            return ParseLambda();
        }


        List<ASTNode?> body = [];
        ASTNode? condition = null;
        while (GetTokenName() != TokenName.KW_End)
        {
            if (GetTokenType() == TokenType.Eof)
            {
                throw BlockEofError(openToken);
            }

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

    private ASTNode? ParseComment()
    {
        MatchType(TokenType.Comment);
        return null;
    }

    private StructNode? ParseStruct(bool isAbstract = false)
    {
        var openToken = token;
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
        StructsDefined.Add(structName);

        List<ASTNode?> methods = [];
        List<(string Name, ASTNode? Initializer)> staticVars = [];
        bool isStatic = false, isPrivate = false, isAbstractMethod = false, isOverride = false;

        while (GetTokenName() != TokenName.KW_End)
        {
            if (GetTokenType() == TokenType.Eof)
            {
                throw BlockEofError(openToken);
            }

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
            else if (MatchName(TokenName.KW_Abstract))
            {
                isAbstractMethod = true;
                continue;
            }
            else if (MatchName(TokenName.KW_Override))
            {
                isOverride = true;
                continue;
            }

            // Static variable declaration: static @varname = expr
            if (isStatic && GetTokenName() == TokenName.KW_This)
            {
                Next();  // consume @
                var varName = "@@" + token.Text;
                Next();  // consume identifier

                // Skip optional type hint: : typename
                if (MatchType(TokenType.Colon))
                {
                    if (GetTokenType() != TokenType.Typename && GetTokenType() != TokenType.Identifier)
                    {
                        throw new SyntaxError(GetErrorToken(), "Expected type name after ':'.");
                    }
                    Next();  // consume type name
                }

                if (GetTokenName() != TokenName.Ops_Assign)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected '=' in static variable declaration.");
                }

                Next();  // consume =
                var initializer = ParseExpression();
                staticVars.Add((varName, initializer));
                isStatic = false;
                continue;
            }

            var statement = ParseStatement() ?? throw new SyntaxError(GetErrorToken(), "Expected a function.");
            if (statement.Type == ASTNodeType.Function)
            {
                var func = (FunctionNode)statement;
                func.IsPrivate = isPrivate;
                func.IsStatic = isStatic;
                func.IsAbstract = isAbstractMethod;
                func.IsOverride = isOverride;

                methods.Add(func);
            }

            isStatic = false;
            isPrivate = false;
            isAbstractMethod = false;
            isOverride = false;
        }

        Next();  // Consume 'end'

        // Outside the struct definition
        structStack.Pop();

        return new StructNode(structName, baseStruct, interfaces, methods, staticVars, isAbstract);
    }

    private EnumNode ParseEnum()
    {
        var openToken = token;
        MatchName(TokenName.KW_Enum);

        if (GetTokenType() != TokenType.Identifier)
        {
            throw new SyntaxError(GetErrorToken(), "Expected identifier for enum name.");
        }

        var enumName = token.Text;
        Next();

        List<(string Name, ASTNode? Value)> members = [];

        while (GetTokenName() != TokenName.KW_End)
        {
            if (GetTokenType() == TokenType.Eof)
            {
                throw BlockEofError(openToken);
            }

            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected identifier for enum member name.");
            }

            var memberName = token.Text;
            Next();

            ASTNode? valueExpr = null;
            if (GetTokenName() == TokenName.Ops_Assign)
            {
                Next(); // consume '='
                valueExpr = ParseExpression();
            }

            members.Add((memberName, valueExpr));
        }

        Next(); // consume 'end'
        return new EnumNode(enumName, members);
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

        // a container for variables
        List<KeyValuePair<string, ASTNode?>> variables = [];

        // a container for type-hints, mapped to variable names.
        Dictionary<string, List<int>> typeHints = [];

        // support single-variable syntax: var name, var name: type, var name: type = value
        if (GetTokenType() != TokenType.LParen)
        {
            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected variable name or '(' after 'var'.");
            }

            var varName = token.Text;
            ASTNode? defaultValue = null;
            Next();  // next token please

            if (MatchType(TokenType.Colon))
            {
                typeHints[varName] = GetTypeNames();
            }

            if (GetTokenType() == TokenType.Operator && GetTokenName() == TokenName.Ops_Assign)
            {
                Next();  // consume '='
                defaultValue = ParseExpression();
            }

            variables.Add(new KeyValuePair<string, ASTNode?>(varName, defaultValue));
            return new VariableNode { Variables = variables, TypeHints = typeHints };
        }

        Next();  // consume '('

        // collect variable declarations
        HashSet<string> seenVars = [];
        while (GetTokenType() != TokenType.RParen)
        {
            // expect an identifier
            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected variable name.");
            }

            // grab the variable name directly from token text
            var varName = token.Text;

            if (!seenVars.Add(varName))
            {
                throw new SyntaxError(GetErrorToken(), $"The variable name `{varName}` is already declared.");
            }

            ASTNode? defaultValue = null;
            Next();  // next token please

            // check for a type-hint after a ':'
            if (MatchType(TokenType.Colon))
            {
                // register type-hint for the variable
                typeHints[varName] = GetTypeNames();
            }

            // check for default value
            if (GetTokenType() == TokenType.Operator &&
                GetTokenName() == TokenName.Ops_Assign)
            {
                Next();  // consume '='
                defaultValue = ParseExpression();
            }

            // add to variables container
            variables.Add(new KeyValuePair<string, ASTNode?>(varName, defaultValue));

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
        var fnKeyword = token;
        MatchType(TokenType.Keyword);  // Consume 'fn'

        // The function name must be on the same line as 'fn'.
        if (token.Span.Line != fnKeyword.Span.Line)
        {
            throw new SyntaxError(fnKeyword, "Expected function name on the same line as 'fn'.");
        }

        var isTypeName = GetTokenType() == TokenType.Typename;
        var isOperator = GetTokenType() == TokenType.Operator;

        if (isOperator && token.Text is "=" or "+=" or "-=" or "*=" or "/=" or "%=" or "**="
                                       or "||=" or "&&=" or "|=" or "&=" or "<<=" or ">>=")
        {
            throw new SyntaxError(fnKeyword, $"'{fnKeyword.Text}' is a reserved keyword and cannot be used as a variable name.");
        }

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

        // Parse parameters
        List<KeyValuePair<string, ASTNode?>> parameters = [];
        Dictionary<string, List<int>> typeHints = [];
        List<int> returnTypeHint = [TypeRegistry.GetType("any")];

        if (isTypeName && GetTokenType() != TokenType.LParen)
        {
            throw new SyntaxError(GetErrorToken(), $"Expected '(' after the identifier `{functionName}`.");
        }

        _generatorMarks.Add(false);
        HashSet<string> seenParams = [];
        var variadicParamName = string.Empty;

        if (GetTokenType() == TokenType.LParen)
        {
            Next();  // Consume '('

            while (GetTokenType() != TokenType.RParen)
            {
                // Variadic parameter: *paramName (must be last)
                if (GetTokenType() == TokenType.Operator && GetTokenName() == TokenName.Ops_Multiply)
                {
                    Next();  // consume '*'
                    if (GetTokenType() != TokenType.Identifier)
                        throw new SyntaxError(GetErrorToken(), "Expected parameter name after '*'.");
                    var varParamName = token.Text;
                    variadicParamName = varParamName;
                    Next();  // consume name
                    if (GetTokenType() != TokenType.RParen)
                        throw new SyntaxError(GetErrorToken(), "Variadic parameter '*' must be the last parameter.");
                    break;
                }

                if (GetTokenType() != TokenType.Identifier)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected parameter name.");
                }

                var paramName = token.Text;

                if (!seenParams.Add(paramName))
                {
                    throw new SyntaxError(GetErrorToken(), $"The parameter name `{paramName}` is already used.");
                }

                ASTNode? defaultValue = null;
                Next();

                if (MatchType(TokenType.Colon))
                {
                    typeHints[paramName] = GetTypeNames();
                }

                // Check for default value
                if (GetTokenType() == TokenType.Operator &&
                    GetTokenName() == TokenName.Ops_Assign)
                {
                    Next();  // Consume '='
                    defaultValue = ParseExpression();
                }

                parameters.Add(new KeyValuePair<string, ASTNode?>(paramName, defaultValue));

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
                returnTypeHint = GetTypeNames();
            }
        }

        // Parse the function body
        var body = ParseSimpleBlock(fnKeyword);

        var isGenerator = _generatorMarks.Count > 0 && _generatorMarks[_generatorMarks.Count - 1];
        if (_generatorMarks.Count > 0)
            _generatorMarks.RemoveAt(_generatorMarks.Count - 1);

        return new FunctionNode
        {
            Name = functionName,
            Parameters = parameters,
            Body = body,
            TypeHints = typeHints,
            ReturnTypeHint = returnTypeHint,
            IsOperatorOverload = isOperator,
            IsGenerator = isGenerator,
            VariadicParamName = variadicParamName
        };
    }

    private ForLoopNode? ParseForLoop()
    {
        var openToken = token;
        MatchName(TokenName.KW_For);  // Consume 'for'

        var valueIteratorName = string.Empty;

        if (GetTokenType() == TokenType.Identifier)
        {
            valueIteratorName = token.Text;
        }

        var valueIterator = ParseIdentifier(false, false);
        ASTNode? indexIterator = null;

        if (MatchType(TokenType.Comma))
        {
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

        var body = ParseSimpleBlock(openToken);

        return new ForLoopNode(valueIterator ?? new IdentifierNode(valueIteratorName))
        {
            IndexIterator = indexIterator,
            DataSet = dataSet,
            Body = body
        };
    }

    private WhileLoopNode? ParseWhileLoop()
    {
        var openToken = token;
        MatchName(TokenName.KW_While);  // Consume 'while'

        var condition = ParseExpression();

        if (!MatchName(TokenName.KW_Do))
        {
            throw new SyntaxError(GetErrorToken(), "Expected 'do' in while-loop.");
        }

        var body = ParseSimpleBlock(openToken);

        return new WhileLoopNode
        {
            Condition = condition,
            Body = body
        };
    }

    private RepeatLoopNode? ParseRepeatLoop()
    {
        var openToken = token;
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

        var body = ParseSimpleBlock(openToken);

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

    private YieldNode? ParseYield()
    {
        var yieldToken = token;
        MatchName(TokenName.KW_Yield);
        var node = new YieldNode { Token = yieldToken };

        if (HasValue())
        {
            node.YieldValue = ParseExpression();
        }

        // Mark the enclosing function as a generator
        if (_generatorMarks.Count > 0)
        {
            _generatorMarks[_generatorMarks.Count - 1] = true;
        }

        return node;
    }

    private ThrowNode? ParseThrow()
    {
        MatchName(TokenName.KW_Throw);
        ASTNode? errorValue = null;
        ASTNode? condition = null;

        if (HasValue())
        {
            errorValue = ParseExpression();
        }

        if (MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            condition = ParseExpression();
        }

        return new ThrowNode(errorValue, condition);
    }

    private ExitNode? ParseExit()
    {
        MatchName(TokenName.KW_Exit);
        ASTNode? exitValue = null;
        ASTNode? condition = null;

        if (HasValue())
        {
            exitValue = ParseExpression();
        }

        if (MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            condition = ParseExpression();
        }

        return new ExitNode(exitValue, condition);
    }

    private BreakNode? ParseBreak()
    {
        MatchName(TokenName.KW_Break);
        ASTNode? condition = null;

        if (MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            condition = ParseExpression();
        }

        return new BreakNode(condition);
    }

    private NextNode? ParseNext()
    {
        MatchName(TokenName.KW_Next);
        ASTNode? condition = null;

        if (MatchName(TokenName.KW_When))
        {
            if (!HasValue())
            {
                throw new SyntaxError(GetErrorToken(), "Expected condition after 'when'.");
            }

            condition = ParseExpression();
        }

        return new NextNode(condition);
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
        var openToken = token;
        MatchName(TokenName.KW_Package);

        if (GetTokenType() is not TokenType.Identifier and
            not TokenType.Typename)
        {
            throw new SyntaxError(GetErrorToken(), "Expected identifier for package name.");
        }

        var packageName = ParseIdentifier(false, true);

        var body = ParseSimpleBlock(openToken);

        if (packageName != null && packageName is IdentifierNode idNode)
        {
            PackagesDefined.Add(idNode.Name);
        }

        return new PackageNode(packageName)
        {
            Body = body
        };
    }

    private CaseNode? ParseCase()
    {
        var openToken = token;
        if (!MatchName(TokenName.KW_Case))
        {
            throw new SyntaxError(GetErrorToken(), "Expected case-statement.");
        }

        var node = new CaseNode();

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

                node.TestValueAlias = ParseIdentifier(false, false);
            }
        }

        while (GetTokenName() != TokenName.KW_End)
        {
            if (GetTokenType() == TokenType.Eof)
            {
                throw BlockEofError(openToken);
            }

            if (MatchName(TokenName.KW_When))
            {
                var caseWhen = new CaseWhenNode();
                if (!HasValue())
                {
                    throw new SyntaxError(GetErrorToken(), "Expected condition or value for case-when.");
                }

                caseWhen.Conditions.Add(ParseCaseWhenCondition());
                while (GetTokenType() == TokenType.Comma)
                {
                    Next();  // consume ','
                    caseWhen.Conditions.Add(ParseCaseWhenCondition());
                }

                MatchType(TokenType.Colon);  // optional ':' after condition(s)

                while (GetTokenName() is not TokenName.KW_When and not TokenName.KW_Else and
                       not TokenName.KW_End)
                {
                    var stmt = ParseStatement(applyWhenGuard: false);
                    if (stmt != null)
                    {
                        caseWhen.Body.Add(stmt);
                    }
                }

                node.WhenNodes.Add(caseWhen);
            }
            else if (MatchName(TokenName.KW_Else))
            {
                MatchType(TokenType.Colon);  // optional ':' after else

                while (GetTokenName() != TokenName.KW_End)
                {
                    if (GetTokenType() == TokenType.Eof)
                    {
                        throw BlockEofError(openToken);
                    }

                    var stmt = ParseStatement();
                    if (stmt != null)
                    {
                        node.ElseBody.Add(stmt);
                    }
                }
            }
            else
            {
                throw new SyntaxError(GetErrorToken(), "Expected 'when' or 'else' in 'case' statement.");
            }
        }

        if (node.WhenNodes.Count == 0 && node.ElseBody.Count == 0)
        {
            throw new SyntaxError(GetErrorToken(), "Expected at least one 'when' clause in 'case' statement.");
        }

        Next();  // Consume 'end'

        return node;
    }

    private bool IsDecoratorPattern()
    {
        // Decorators are only valid outside struct definitions.
        if (structStack.Count > 0) return false;

        // Must be @ followed by an identifier.
        var afterAt = Peek();  // position +1
        if (afterAt.Type != TokenType.Identifier) return false;

        // Determine the offset after the decorator name, accounting for pkg::name.
        int nameEnd = 2;
        if (PeekAt(2).Type == TokenType.Qualifier && PeekAt(3).Type == TokenType.Identifier)
            nameEnd = 4;  // @pkg::name

        var afterIdent = PeekAt(nameEnd);

        // @name fn ... or @name @...
        if (afterIdent.Name == TokenName.KW_Method || afterIdent.Name == TokenName.KW_This)
            return true;

        // @name(args...) fn ... or @name(args...) @...
        if (afterIdent.Type == TokenType.LParen)
        {
            int depth = 1;
            int pos = stream.Position + nameEnd + 1;
            while (pos < stream.Size && depth > 0)
            {
                var t = stream.At(pos++);
                if (t.Type == TokenType.LParen) depth++;
                else if (t.Type == TokenType.RParen) depth--;
            }
            if (pos < stream.Size)
            {
                var afterArgs = stream.At(pos);
                return afterArgs.Name == TokenName.KW_Method || afterArgs.Name == TokenName.KW_This;
            }
        }

        // @name.member fn ... or @name.member(args...) fn ...
        if (afterIdent.Type == TokenType.Dot && PeekAt(nameEnd + 1).Type == TokenType.Identifier)
        {
            int memberEnd = nameEnd + 2;
            var afterMember = PeekAt(memberEnd);

            if (afterMember.Name == TokenName.KW_Method || afterMember.Name == TokenName.KW_This)
                return true;

            if (afterMember.Type == TokenType.LParen)
            {
                int depth = 1;
                int pos = stream.Position + memberEnd + 1;
                while (pos < stream.Size && depth > 0)
                {
                    var t = stream.At(pos++);
                    if (t.Type == TokenType.LParen) depth++;
                    else if (t.Type == TokenType.RParen) depth--;
                }
                if (pos < stream.Size)
                {
                    var afterArgs = stream.At(pos);
                    return afterArgs.Name == TokenName.KW_Method || afterArgs.Name == TokenName.KW_This;
                }
            }
        }

        return false;
    }

    private DecoratedFunctionNode? ParseDecoratedFunction()
    {
        var openToken = token;
        var decorators = new List<(ASTNode? Expr, List<ASTNode?> ExtraArgs)>();

        while (GetTokenName() == TokenName.KW_This)
        {
            Next();  // consume '@'

            if (GetTokenType() != TokenType.Identifier)
                throw new SyntaxError(GetErrorToken(), "Expected identifier after '@'.");

            var nameToken = token;
            var baseName = token.Text;
            Next();  // consume first name part

            ASTNode? decoratorExpr;

            // Support package-qualified decorators: @pkg::name
            if (GetTokenType() == TokenType.Qualifier)
            {
                Next();  // consume '::'
                if (GetTokenType() != TokenType.Identifier)
                    throw new SyntaxError(GetErrorToken(), "Expected identifier after '::'.");
                decoratorExpr = new IdentifierNode(baseName + "::" + token.Text) { Token = nameToken };
                Next();  // consume member name
            }
            // Support member-access decorators: @x.name
            else if (GetTokenType() == TokenType.Dot)
            {
                Next();  // consume '.'
                if (GetTokenType() != TokenType.Identifier)
                    throw new SyntaxError(GetErrorToken(), "Expected identifier after '.' in decorator.");
                var memberName = token.Text;
                Next();  // consume member name
                decoratorExpr = new MemberAccessNode(
                    new IdentifierNode(baseName) { Token = nameToken }, memberName) { Token = nameToken };
            }
            else
            {
                decoratorExpr = new IdentifierNode(baseName) { Token = nameToken };
            }

            List<ASTNode?> extraArgs = [];
            if (GetTokenType() == TokenType.LParen && token.Span.Line == Previous().Span.Line)
            {
                extraArgs = CollectCallArguments();
            }

            decorators.Add((decoratorExpr, extraArgs));
        }

        if (GetTokenName() != TokenName.KW_Method)
            throw new SyntaxError(GetErrorToken(), "Expected 'fn' after decorator(s).");

        var funcNode = ParseFunction() ?? throw new SyntaxError(GetErrorToken(), "Expected function definition.");

        return new DecoratedFunctionNode
        {
            Function = funcNode,
            Decorators = decorators,
            Token = openToken
        };
    }

    private ASTNode? ParseCaseWhenCondition()
    {
        var expr = ParseExpression();

        if (GetTokenType() == TokenType.Range || GetTokenName() == TokenName.KW_To)
        {
            Next();  // consume '..' or 'to'
            var rangeEnd = ParseExpression();
            return new RangeLiteralNode(expr, rangeEnd) { Token = expr?.Token ?? token };
        }

        return expr;
    }

    private IfNode? ParseIf()
    {
        var openToken = token;
        if (!MatchName(TokenName.KW_If))
        {
            throw new SyntaxError(GetErrorToken(), "Expected if-statement.");
        }

        if (!HasValue() || token.Span.Line != openToken.Span.Line)
        {
            throw new SyntaxError(openToken, "Expected condition on the same line as 'if'.");
        }

        var node = new IfNode
        {
            Condition = ParseExpression()
        };

        int blocks = 1;
        var building = TokenName.KW_If;

        while (stream.CanRead && blocks > 0)
        {
            if (GetTokenType() == TokenType.Eof)
            {
                throw BlockEofError(openToken);
            }

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
                    bool hasCatchParen = MatchType(TokenType.LParen);
                    if (hasCatchParen || (GetTokenType() == TokenType.Identifier && token.Span.Line == Previous().Span.Line))
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

                        if (hasCatchParen && !MatchType(TokenType.RParen))
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
            if (GetTokenType() == TokenType.Operator && GetTokenName() == TokenName.Ops_Multiply)
            {
                var splatToken = token;
                Next();  // consume '*'
                arguments.Add(new SplatNode(ParseExpression()) { Token = splatToken });
            }
            else if (GetTokenType() == TokenType.Identifier && (Peek().Name == TokenName.Ops_Assign || Peek().Type == TokenType.Colon))
            {
                var argToken = token;
                var name = GetTokenText();
                Next();  // consume identifier
                Next();  // consume '=' or ':'
                var namedArg = new NamedArgumentNode(name, ParseExpression()) { Token = argToken };
                arguments.Add(namedArg);
            }
            else
            {
                arguments.Add(ParseExpression());
            }

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

    private List<ASTNode?> ParseSimpleBlock(Token openToken)
    {
        List<ASTNode?> body = [];
        while (GetTokenName() != TokenName.KW_End)
        {
            if (GetTokenType() == TokenType.Eof)
            {
                throw BlockEofError(openToken);
            }

            var stmt = ParseStatement();
            if (stmt != null)
            {
                body.Add(stmt);
            }
        }
        Next();  // Consume 'end'
        return body;
    }

    private LambdaNode? ParseLambda()
    {
        /*
        When coming from `with`: consume otherwise assume: do (params) => statement / do (params) statements end
        This is for backwards compatibility with older syntax.
        */
        var openToken = token;
        var assumedDoParse = GetTokenType() != TokenType.Lambda;
        if (!assumedDoParse)
        {
            Next(); // consume
        }

        Dictionary<string, List<int>> typeHints = [];
        List<int> returnTypeHint = [TypeRegistry.GetType("any")];
        HashSet<string> seenParams = [];
        var variadicParamName = string.Empty;

        // Parse parameters
        List<KeyValuePair<string, ASTNode?>> parameters = [];
        if (GetTokenType() == TokenType.LParen)
        {
            Next();  // Consume '('

            while (GetTokenType() != TokenType.RParen)
            {
                // Variadic parameter: *paramName (must be last)
                if (GetTokenType() == TokenType.Operator && GetTokenName() == TokenName.Ops_Multiply)
                {
                    Next();  // consume '*'
                    if (GetTokenType() != TokenType.Identifier)
                        throw new SyntaxError(GetErrorToken(), "Expected parameter name after '*'.");
                    var varParamName = token.Text;
                    variadicParamName = varParamName;
                    Next();  // consume name
                    if (GetTokenType() != TokenType.RParen)
                        throw new SyntaxError(GetErrorToken(), "Variadic parameter '*' must be the last parameter.");
                    break;
                }

                if (GetTokenType() != TokenType.Identifier)
                {
                    throw new SyntaxError(GetErrorToken(), "Expected parameter name.");
                }

                var paramName = token.Text;

                if (!seenParams.Add(paramName))
                {
                    throw new SyntaxError(GetErrorToken(), $"The parameter name '{paramName}' is already used.");
                }

                ASTNode? defaultValue = null;
                Next();

                // Check for type hint.
                if (MatchType(TokenType.Colon))
                {
                    typeHints[paramName] = GetTypeNames();
                }

                // Check for default value
                if (GetTokenType() == TokenType.Operator &&
                    GetTokenName() == TokenName.Ops_Assign)
                {
                    Next();  // Consume '='
                    defaultValue = ParseExpression();
                }

                parameters.Add(new KeyValuePair<string, ASTNode?>(paramName, defaultValue));

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
                returnTypeHint = GetTypeNames();
            }
        }

        // with (params) do ...
        if (!assumedDoParse && !MatchName(TokenName.KW_Do))
        {
            throw new SyntaxError(GetErrorToken(), "Expected 'do' in lambda expression.");
        }

        // Parse the lambda body
        List<ASTNode?> body = [];

        // Look for the arrow
        if (GetTokenType() == TokenType.Arrow)
        {
            Next(); // consume the arrow

            // If there is an arrow, we will only allow a single statement.
            // This is the end of the lambda decl.

            var stmt = ParseStatement();
            if (stmt != null)
            {
                body.Add(stmt);
            }

            // Optionally consume 'end' as arrow lambdas don't require it, but
            // users may write `do (x) => expr end` (symmetric with multi-line form).
            // Without this, a trailing 'end' would be left in the stream and cause
            // a parse error in the surrounding call argument list.
            MatchName(TokenName.KW_End);
        }
        else
        {
            // Read until `end`.
            body = ParseSimpleBlock(openToken);
        }

        return new LambdaNode
        {
            Parameters = parameters,
            Body = body,
            TypeHints = typeHints,
            ReturnTypeHint = returnTypeHint,
            VariadicParamName = variadicParamName
        };
    }

    private PrintNode? ParsePrint()
    {
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

        var value = Value.Create(token.Value);
        Next();  // Consume literal
        return new LiteralNode(value);
    }

    private InterpolationNode ParseInterpolation()
    {
        var interpNode = new InterpolationNode { Token = token };
        var text = GetTokenText();
        Next();  // consume the Interpolation token

        var sb = new System.Text.StringBuilder();

        for (int i = 0; i < text.Length; i++)
        {
            var c = text[i];

            if (c == '$' && i + 1 < text.Length && text[i + 1] == '{')
            {
                // \${ escaped: treat as literal "${...}"
                if (sb.Length > 0 && sb[^1] == '\\')
                {
                    sb.Remove(sb.Length - 1, 1);
                    sb.Append("${");
                    i += 2;  // skip '$' and '{'
                    int depth = 1;
                    while (i < text.Length && depth > 0)
                    {
                        char ci = text[i];
                        
                        if (ci == '{')
                        {
                            depth++;
                            sb.Append(ci);
                        }
                        else if (ci == '}')
                        {
                            if (--depth > 0)
                            {
                                sb.Append(ci);
                            }
                        }
                        else 
                        {
                            sb.Append(ci);
                        }
                        
                        i++;
                    }

                    sb.Append('}');
                    --i;
                    continue;
                }

                // Flush accumulated literal segment
                interpNode.Parts.Add(new LiteralNode(Value.CreateString(sb.ToString())) { Token = interpNode.Token });
                sb.Clear();

                int dollarIdx = i;
                i += 2;  // skip '$' and '{'

                // Collect expression until matching '}'
                // Nested string literals are scanned verbatim so that a '"' or '\'' inside
                // the expression (e.g. ${foo("bar")}) doesn't confuse the brace counter.
                int braces = 1;
                while (i < text.Length && braces > 0)
                {
                    char ci = text[i];

                    if (ci == '{')
                    {
                        braces++;
                        sb.Append(ci);
                    }
                    else if (ci == '}')
                    {
                        if (--braces > 0)
                        {
                            sb.Append(ci);
                        }
                    }
                    else if (ci == '"' || ci == '\'')
                    {
                        // Scan nested string literal verbatim until the matching closing quote
                        char quote = ci;
                        sb.Append(ci);
                        i++;
                        bool nestedEsc = false;
                        while (i < text.Length)
                        {
                            char nc = text[i];
                            sb.Append(nc);
                            i++;
                            if (nestedEsc)
                            {
                                nestedEsc = false;
                            }
                            else if (nc == '\\' && quote == '"')
                            {
                                nestedEsc = true;
                            }
                            else if (nc == quote)
                            {
                                break;
                            }
                        }
                        // i already points past the closing quote; skip the trailing i++
                        continue;
                    }
                    else
                    {
                        sb.Append(ci);
                    }

                    i++;
                }

                --i;  // compensate for loop's ++i

                var code = sb.ToString();
                sb.Clear();

                if (!string.IsNullOrEmpty(code))
                {
                    // Compute source position of the expression inside ${...}.
                    // i points one past the closing '}', so the '$' was at (i - code.Length - 2 - 1).
                    // Simpler: we captured dollarIdx before advancing i.
                    int exprLine = interpNode.Token.Span.Line;
                    int exprCol  = interpNode.Token.Span.Pos + 1 + dollarIdx + 2;  // past `"`, prefix chars, `${`
                    using var lex = new Lexer(interpNode.Token.Span.File, code, exprLine, exprCol);
                    var subStream = lex.GetTokenStream();
                    var savedStream = stream;
                    var savedToken = token;
                    var exprNode = ParseExpressionFromStream(subStream);
                    stream = savedStream;
                    token = savedToken;
                    
                    if (exprNode != null)
                    {
                        interpNode.Parts.Add(exprNode);
                    }
                }
            }
            else
            {
                sb.Append(c);
            }
        }

        // Flush remaining literal segment
        if (sb.Length > 0)
        {
            interpNode.Parts.Add(new LiteralNode(Value.CreateString(sb.ToString())) { Token = interpNode.Token });
        }

        return interpNode;
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
                not TokenType.Literal and
                not TokenType.Operator)
            {
                throw new SyntaxError(GetErrorToken(), "Invalid expression for hashmap key.");
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
                not ASTNodeType.Self and
                not ASTNodeType.FunctionCall and
                not ASTNodeType.BinaryOperation and
                not ASTNodeType.MethodCall and
                not ASTNodeType.MemberAccess and
                not ASTNodeType.Interpolation and
                not ASTNodeType.Index)
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

    private ASTNode? ParseSuperCall()
    {
        var superToken = token.Clone();
        Next(); // consume 'super'

        if (GetTokenType() != TokenType.Dot)
            throw new SyntaxError(GetErrorToken(), "Expected '.' after 'super'.");
        Next(); // consume '.'

        if (GetTokenType() != TokenType.Identifier)
            throw new SyntaxError(GetErrorToken(), "Expected method name after 'super.'.");

        var methodName = token.Text;
        Next(); // consume method name

        if (GetTokenType() != TokenType.LParen)
            throw new SyntaxError(GetErrorToken(), "Expected '(' after super method name.");

        var arguments = CollectCallArguments();
        return new SuperCallNode(methodName, arguments) { Token = superToken };
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

    private MethodCallNode? ParseMethodCall(ASTNode? obj, string methodName, TokenName type)
    {
        var arguments = CollectCallArguments();
        return new MethodCallNode(obj, methodName, type, arguments);
    }

    private MemberAssignmentNode? ParseMemberAssignment(ASTNode? obj, string memberName)
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
            Next();

            assignment.Left.Add(
                new IdentifierNode(identifierName));
        }

        if (MatchName(TokenName.Ops_Unpack))
        {
            Console.Error.WriteLine($"Deprecation warning: '=<' is deprecated. Use '=' for unpacking instead.");
        }
        else if (!MatchName(TokenName.Ops_Assign))
        {
            throw new SyntaxError(GetErrorToken(), "Expected '=' in pack assignment.");
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

            if (MatchName(TokenName.KW_Else))
            {
                if (!HasValue())
                {
                    throw new SyntaxError(GetErrorToken(), "Expected expression after 'else'.");
                }

                assignment.ElseInitializer = ParseExpression();
            }
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
        else if (GetTokenType() == TokenType.LBracket && token.Span.Line == Previous().Span.Line)
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

        // Static self: @@varname
        if (GetTokenName() == TokenName.KW_StaticSelf)
        {
            Next();  // consume @@

            if (GetTokenType() != TokenType.Identifier)
            {
                throw new SyntaxError(GetErrorToken(), "Expected identifier after '@@'.");
            }

            var staticVarName = "@@" + token.Text;
            Next();  // consume identifier

            ASTNode? staticNode = new StaticSelfNode(staticVarName) { Token = idToken };

            if (GetTokenType() == TokenType.Operator && IsAssignmentOperator())
            {
                staticNode = ParseAssignment(staticNode, staticVarName);
            }

            if (staticNode != null)
            {
                staticNode.Token = idToken;
            }

            return staticNode;
        }

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
            case TokenType.LParen when token.Span.Line == Previous().Span.Line:
                node = ParseFunctionCall(identifierName, type);
                break;
            case TokenType.LBracket when token.Span.Line == Previous().Span.Line:
                node = ParseIndexing(node);
                break;
            case TokenType.Operator when IsAssignmentOperator():
                node = ParseAssignment(node, identifierName);
                break;
            case TokenType.Qualifier when Peek().Type is TokenType.Identifier or TokenType.Typename:
                node = ParseQualifiedIdentifier(identifierName);
                break;
            case TokenType.Comma when !packed && IsPackAssignAhead():
                node = ParsePackAssignment(node);
                break;
            default:
                if (isTypeName && !lenient)
                {
                    throw new SyntaxError(GetErrorToken(), $"Expected '(' or '::' after the identifier `{identifierName}`.");
                }
                else if (!isInstance)
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

        while (stream.CanRead && Previous().Span.Line == token.Span.Line && (GetTokenName() == TokenName.Ops_Or || GetTokenName() == TokenName.Ops_NullCoalesce))
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

        while (stream.CanRead && Previous().Span.Line == token.Span.Line && GetTokenName() == TokenName.Ops_And)
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

        while (stream.CanRead && Previous().Span.Line == token.Span.Line && GetTokenName() == TokenName.Ops_BitwiseOr)
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

        while (stream.CanRead && Previous().Span.Line == token.Span.Line && GetTokenName() == TokenName.Ops_BitwiseXor)
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

        while (stream.CanRead && Previous().Span.Line == token.Span.Line && GetTokenName() == TokenName.Ops_BitwiseAnd)
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

        while (stream.CanRead && Previous().Span.Line == token.Span.Line && IsEqualityOperator())
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

        while (stream.CanRead && Previous().Span.Line == token.Span.Line && IsComparisonOperator())
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

        while (stream.CanRead && Previous().Span.Line == token.Span.Line && IsBitwiseOperator())
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
        while (stream.CanRead && Previous().Span.Line == token.Span.Line && IsAdditiveOperator())
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

        while (stream.CanRead && Previous().Span.Line == token.Span.Line && IsMultiplicativeOperator())
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

            case TokenType.Interpolation:
                node = ParseInterpolation();
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

        while (true)
        {
            if (GetTokenType() == TokenType.Dot)
            {
                node = ParseMemberAccess(node);
            }
            else if (GetTokenType() == TokenType.LBracket && token.Span.Line == Previous().Span.Line)
            {
                node = ParseIndexing(node);
            }
            else if (GetTokenType() == TokenType.LParen && token.Span.Line == Previous().Span.Line)
            {
                node = ParseLambdaCall(node);
            }
            else
            {
                break;
            }
        }

        if (node != null)
        {
            node.Token = nodeToken;
        }

        return node;
    }
}