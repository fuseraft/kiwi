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

        // Like a string, a list repeated zero or fewer times is empty.
        if (multiplier <= 0 || list.Count == 0)
        {
            return [];
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