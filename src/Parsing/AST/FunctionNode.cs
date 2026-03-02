using kiwi.Typing;

namespace kiwi.Parsing.AST;

public class FunctionNode() : ASTNode(ASTNodeType.Function)
{
    public string Name { get; set; } = string.Empty;
    public List<KeyValuePair<string, ASTNode?>> Parameters { get; set; } = [];
    public List<ASTNode?> Body { get; set; } = [];
    public Dictionary<string, List<int>> TypeHints { get; set; } = [];
    public List<int> ReturnTypeHint { get; set; } = [TypeRegistry.GetType("any")];
    public bool IsOperatorOverload { get; set; }
    public bool IsStatic { get; set; }
    public bool IsPrivate { get; set; }
    public bool IsGenerator { get; set; }
    public bool IsAbstract { get; set; }
    public bool IsOverride { get; set; }

    public LambdaNode ToLambda()
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

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        Print($"Function: `{ASTTracer.Unmangle(Name)}`");

        if (IsStatic || IsPrivate || IsAbstract || IsOverride)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("Modifiers:");
            ASTTracer.PrintDepth(2 + depth);
            Print($"Private: {IsPrivate}, Static: {IsStatic}, Abstract: {IsAbstract}, Override: {IsOverride}");
        }

        ASTTracer.PrintDepth(1 + depth);
        Print($"ReturnType: {string.Join("|", ReturnTypeHint.Select(TypeRegistry.GetTypeName))}");

        ASTTracer.PrintDepth(1 + depth);
        Print("Parameters: ");
        foreach (var param in Parameters)
        {
            ASTTracer.PrintDepth(2 + depth);
            Print($"Identifier: `{ASTTracer.Unmangle(param.Key)}`");

            if (TypeHints.TryGetValue(param.Key, out List<int>? value))
            {
                ASTTracer.PrintDepth(3 + depth);
                Print($"ParameterType: {string.Join("|", value.Select(TypeRegistry.GetTypeName))}");
            }

            if (param.Value == null)
            {
                continue;
            }

            ASTTracer.PrintDepth(2 + depth);
            Print("Default: ");
            param.Value.Print(3 + depth);
        }

        ASTTracer.PrintDepth(1 + depth);
        Print("Statements:");
        foreach (var stmt in Body)
        {
            stmt?.Print(2 + depth);
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

        return new FunctionNode
        {
            Name = Name,
            Parameters = clonedParameters,
            Body = clonedBody,
            IsStatic = IsStatic,
            IsPrivate = IsPrivate,
            IsGenerator = IsGenerator,
            IsAbstract = IsAbstract,
            IsOverride = IsOverride,
            TypeHints = TypeHints,
            ReturnTypeHint = ReturnTypeHint,
            Token = Token
        };
    }
}