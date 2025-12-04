using kiwi.Typing;

namespace kiwi.Parsing.AST;

public class LambdaNode : ASTNode
{
    public LambdaNode()
        : base(ASTNodeType.Lambda) { }

    public List<KeyValuePair<string, ASTNode?>> Parameters { get; set; } = [];
    public List<ASTNode?> Body { get; set; } = [];
    public Dictionary<string, TokenName> TypeHints { get; set; } = [];
    public TokenName ReturnTypeHint { get; set; } = TokenName.Types_Any;

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ASTTracer.PrintDepth(1 + depth);
        Console.WriteLine($"ReturnType: {Serializer.GetTypenameString(ReturnTypeHint)}");

        ASTTracer.PrintDepth(1 + depth);
        Console.WriteLine("Parameters: ");
        foreach (var param in Parameters)
        {
            ASTTracer.PrintDepth(2 + depth);
            Console.WriteLine($"Identifier: `{ASTTracer.Unmangle(param.Key)}`");

            if (TypeHints.ContainsKey(param.Key))
            {
                ASTTracer.PrintDepth(2 + depth);
                var typeHint = TypeHints[param.Key];
                Console.WriteLine($"ParameterType: {Serializer.GetTypenameString(typeHint)}");
            }

            if (param.Value == null)
            {
                continue;
            }

            ASTTracer.PrintDepth(2 + depth);
            Console.WriteLine("Default: ");
            param.Value.Print(1 + depth);
        }

        ASTTracer.PrintDepth(depth);
        Console.WriteLine("Statements:");
        foreach (var stmt in Body)
        {
            stmt?.Print(1 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<KeyValuePair<string, ASTNode?>> clonedParameters = [];
        foreach (var param in Parameters)
        {
            KeyValuePair<string, ASTNode?> item = new(param.Key, param.Value?.Clone());
            clonedParameters.Add(item);
        }

        List<ASTNode?> clonedBody = [];
        foreach (var stmt in Body)
        {
            clonedBody.Add(stmt?.Clone());
        }

        return new LambdaNode
        {
            Parameters = clonedParameters,
            Body = clonedBody,
            TypeHints = TypeHints,
            ReturnTypeHint = ReturnTypeHint,
            Token = Token
        };
    }
}