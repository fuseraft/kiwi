namespace kiwi.Parsing.AST;

public class SliceNode(ASTNode? slicedObject = null, ASTNode? startExpression = null, ASTNode? stopExpression = null, ASTNode? stepExpression = null) : ASTNode(ASTNodeType.Slice)
{
    public ASTNode? SlicedObject { get; } = slicedObject;
    public ASTNode? StartExpression { get; } = startExpression;
    public ASTNode? StopExpression { get; } = stopExpression;
    public ASTNode? StepExpression { get; } = stepExpression;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);

        if (SlicedObject != null)
        {
            Console.WriteLine("Slice on object:");
            SlicedObject.Print(1 + depth);
        }

        if (StartExpression != null)
        {
            ASTTracer.PrintDepth(depth);
            Console.WriteLine("Slice start:");
            StartExpression.Print(1 + depth);
        }

        if (StopExpression != null)
        {
            ASTTracer.PrintDepth(depth);
            Console.WriteLine("Slice stop:");
            StopExpression.Print(1 + depth);
        }

        if (StepExpression != null)
        {
            ASTTracer.PrintDepth(depth);
            Console.WriteLine("Slice step:");
            StepExpression.Print(1 + depth);
        }
    }

    public override ASTNode Clone() => new SliceNode(SlicedObject?.Clone(), StartExpression?.Clone(), StopExpression?.Clone(), StepExpression?.Clone())
    {
        Token = Token
    };
}