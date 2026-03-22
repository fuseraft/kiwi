using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;
using kiwi.Parsing.AST;
using kiwi.VM;

namespace kiwi.Runtime.Builtin.Handler;

public class SerializerBuiltinHandler
{
    public static Value Execute(Token token, TokenName op, List<Value> args)
    {
        return op switch
        {
            TokenName.Builtin_Serializer_Deserialize => Deserialize(token, args),
            TokenName.Builtin_Serializer_Serialize => Serialize(token, args),
            _ => throw new InvalidOperationError(token, "Come back later."),
        };
    }

    private static Value Deserialize(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SerializerBuiltin.Deserialize, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SerializerBuiltin.Deserialize, 0, args[0]);

        var input = args[0].GetString();
        using Lexer lexer = new(token.Span.File, input);
        var ast = new Parser(true).ParseTokenStream(lexer.GetTokenStream(), true);
        var chunk = Compiler.CompileExpression((ProgramNode)ast);
        var savedVM = KiwiVM.Current;
        try   { return new KiwiVM(new Interpreter()).Execute(chunk); }
        finally { KiwiVM.Current = savedVM; }
    }

    private static Value Serialize(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SerializerBuiltin.Deserialize, 1, args.Count);
        return Value.CreateString(Serializer.Serialize(args[0]));
    }
}