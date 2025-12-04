using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Util;

public struct SliceUtil
{
    public static Value StringSlice(Token token, SliceIndex slice, string s)
    {
        List<Value> list = [];

        var temp = string.Empty;
        foreach (var c in s.ToCharArray())
        {
            temp = string.Empty + c;
            list.Add(Value.CreateString(temp));
        }

        var sliced = ListSlice(token, slice, list);

        if (sliced.IsList())
        {
            System.Text.StringBuilder sv = new();

            foreach (var v in sliced.GetList())
            {
                sv.Append(Serializer.Serialize(v));
            }

            return Value.CreateString(sv.ToString());
        }

        return Value.CreateString(Serializer.Serialize(sliced));
    }

    public static Value ListSlice(Token token, SliceIndex slice, List<Value> list)
    {
        if (!slice.IndexOrStart.IsInteger())
        {
            throw new IndexError(token, "Start index must be an integer.");
        }
        else if (!slice.StopIndex.IsInteger())
        {
            throw new IndexError(token, "Stop index must be an integer.");
        }
        else if (!slice.StepValue.IsInteger())
        {
            throw new IndexError(token, "Step value must be an integer.");
        }

        var start = (int)slice.IndexOrStart.GetInteger();
        var stop = (int)slice.StopIndex.GetInteger();
        var step = (int)slice.StepValue.GetInteger();

        // adjust negative indices
        int listSize = list.Count;
        if (start < 0)
        {
            start = start + listSize > 0 ? start + listSize : 0;
        }

        if (stop < 0)
        {
            stop += listSize;
        }
        else
        {
            stop = stop < listSize ? stop : listSize;
        }

        // special case for reverse slicing
        stop = step < 0 && stop == listSize ? -1 : stop;

        List<Value> slicedList = [];

        if (step < 0)
        {
            for (int i = start == 0 ? listSize - 1 : start; i >= stop; i += step)
            {
                // prevent out-of-bounds access
                if (i < 0 || i >= listSize)
                {
                    break;
                }

                slicedList.Add(list[i]);
            }
        }
        else
        {
            for (int i = start; i < stop; i += step)
            {
                // prevent out-of-bounds access
                if (i >= listSize)
                {
                    break;
                }

                slicedList.Add(list[i]);
            }
        }

        return Value.CreateList(slicedList);
    }

    public static void UpdateListSlice(Token token, bool insertOp, ref List<Value> targetList, SliceIndex slice, List<Value> rhsValues)
    {
        if (!slice.IndexOrStart.IsInteger())
        {
            throw new IndexError(token, "Start index must be an integer.");
        }
        else if (!slice.StopIndex.IsInteger())
        {
            throw new IndexError(token, "Stop index must be an integer.");
        }
        else if (!slice.StepValue.IsInteger())
        {
            throw new IndexError(token, "Step value must be an integer.");
        }

        var start = (int)slice.IndexOrStart.GetInteger();
        var stop = (int)slice.StopIndex.GetInteger();
        var step = (int)slice.StepValue.GetInteger();

        // this is a single element assignment
        if (!slice.IsSlice && insertOp)
        {
            stop = start;
        }

        // convert negative indices and adjust ranges
        int listSize = targetList.Count;
        int rhsSize = rhsValues.Count;

        // adjust negative indices to be from the end
        start += start < 0 ? listSize : 0;
        stop += stop < 0 ? listSize : 0;

        // clamp indices within the range [0, listSize]
        start = start < 0 ? 0 : start;
        stop = stop > listSize ? listSize : stop;

        // special case for reverse slicing
        stop = step < 0 && stop == listSize ? -1 : stop;

        if (step == 1)
        {
            // simple case: step is 1
            if (start >= stop)
            {
                int removeCount = Math.Max(0, stop - start); // ensure non-negative
                if (removeCount > 0)
                {
                    targetList.RemoveRange(start, removeCount);
                }

                targetList.InsertRange(start, rhsValues);
            }
            else
            {
                for (int i = 0; i < rhsSize; i++)
                {
                    if (start + i >= 0 && start + i < targetList.Count)
                    {
                        targetList[start + i] = rhsValues[i];
                    }
                    else
                    {
                        break; // avoid going out of bounds
                    }
                }
            }
        }
        else
        {
            // complex case: step != 1
            int rhsIndex = 0;
            for (int i = start; i != stop && rhsIndex < rhsSize; i += step)
            {
                if (step > 0 && i < listSize || step < 0 && i >= 0)
                {
                    targetList[i] = rhsValues[rhsIndex++];
                }
                else
                {
                    break;  // avoid going out of bounds
                }
            }
        }
    }
}