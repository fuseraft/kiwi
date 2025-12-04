using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Operation;

public struct ListOp
{
    public static List<Value> ListMul(Token token, ref Value left, ref Value right)
    {
        var list = left.GetList();
        var multiplier = right.GetInteger();

        if (multiplier < 1)
        {
            throw new SyntaxError(token, "List multiplier must be a positive non-zero integer.");
        }

        if (list.Count == 0)
        {
            throw new SyntaxError(token, "Cannot multiply an empty list.");
        }

        List<Value> newList = [];

        for (int i = 0; i < multiplier; ++i)
        {
            foreach (var item in list)
            {
                newList.Add(item.Clone());
            }
        }

        return newList;
    }
}