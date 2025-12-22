namespace kiwi.Typing;

public class InstanceRef
{
    public string Identifier { get; set; } = string.Empty;
    public string StructName { get; set; } = string.Empty;
    public Dictionary<string, Value> InstanceVariables { get; set; } = [];

    public bool HasVariable(string name)
    {
        return InstanceVariables.ContainsKey(name);
    }
}

public class LambdaRef
{
    public string Identifier { get; set; } = string.Empty;
}

public class StructRef
{
    public string Identifier { get; set; } = string.Empty;
}

public class NullRef { }

public class SliceIndex(Value indexOrStart, Value stopIndex, Value stepValue)
{
    public Value IndexOrStart { get; set; } = indexOrStart;
    public Value StopIndex { get; set; } = stopIndex;
    public Value StepValue { get; set; } = stepValue;
    public bool IsSlice { get; set; }
};

public class Value(object value, ValueType type = ValueType.None) : IComparable<Value>, IComparable
{
    public object Value_ { get; set; } = value;
    public ValueType Type { get; set; } = type;

    public static Value Create(object value)
    {
        if (value is Value v)
        {
            return v;
        }

        return value switch
        {
            null => CreateNull(),
            long => CreateInteger(value),
            double => CreateFloat(value),
            bool => CreateBoolean(value),
            string => CreateString(value),
            DateTime => CreateDate(value),
            List<Value> => CreateList(value),
            Dictionary<Value, Value> => CreateHashmap(value),
            InstanceRef => CreateObject(value),
            LambdaRef => CreateLambda(value),
            StructRef => CreateStruct(value),
            NullRef => CreateNull(),
            _ => CreateNull()
        };
    }

    public static readonly Value EmptyString = CreateString(string.Empty);
    public static readonly Value Default = new(0L, ValueType.Integer);
    public static readonly Value True = CreateBoolean(true);
    public static readonly Value False = CreateBoolean(false);
    public static Value CreateDate(DateTime value) => new(value, ValueType.Date);
    public static Value CreateDate(object value) => new(value, ValueType.Date);
    public static Value CreateInteger(long value) => new(value, ValueType.Integer);
    public static Value CreateInteger(object value) => new(value, ValueType.Integer);
    public static Value CreateFloat(double value) => new(value, ValueType.Float);
    public static Value CreateFloat(object value) => new(value, ValueType.Float);
    public static Value CreateBoolean(bool value) => new(value, ValueType.Boolean);
    public static Value CreateBoolean(object value) => new(value, ValueType.Boolean);
    public static Value CreateString(string value) => new(value, ValueType.String);
    public static Value CreateString(object value) => new(value, ValueType.String);
    public static Value CreateString(char value) => new($"value", ValueType.String);
    public static Value CreateList(List<Value> value) => new(value, ValueType.List);
    public static Value CreateList(object value) => new(value, ValueType.List);
    public static Value CreateList() => new(new List<Value>(), ValueType.List);
    public static Value CreateHashmap(Dictionary<Value, Value> value) => new(value, ValueType.Hashmap);
    public static Value CreateHashmap(object value) => new(value, ValueType.Hashmap);
    public static Value CreateHashmap() => new(new Dictionary<Value, Value>(), ValueType.Hashmap);
    public static Value CreateNull(NullRef value) => new(value, ValueType.None);
    public static Value CreateNull(object value) => new(value, ValueType.None);
    public static Value CreateNull() => new(new NullRef(), ValueType.None);
    public static Value CreateObject(InstanceRef value) => new(value, ValueType.Object);
    public static Value CreateObject(object value) => new(value, ValueType.Object);
    public static Value CreateLambda(LambdaRef value) => new(value, ValueType.Lambda);
    public static Value CreateLambda(object value) => new(value, ValueType.Lambda);
    public static Value CreateStruct(StructRef value) => new(value, ValueType.Struct);
    public static Value CreateStruct(object value) => new(value, ValueType.Struct);
    public static Value CreatePointer(IntPtr value) => new(value, ValueType.Pointer);

    public double GetNumber()
    {
        if (IsInteger())
        {
            return (double)GetInteger();
        }
        else if (IsFloat())
        {
            return GetFloat();
        }

        return 0D;
    }

    public long GetInteger() => (long)Value_;
    public double GetFloat() => (double)Value_;
    public bool GetBoolean() => (bool)Value_;
    public string GetString() => (string)Value_;
    public DateTime GetDate() => (DateTime)Value_;
    public List<Value> GetList() => (List<Value>)Value_;
    public Dictionary<Value, Value> GetHashmap() => (Dictionary<Value, Value>)Value_;
    public InstanceRef GetObject() => (InstanceRef)Value_;
    public LambdaRef GetLambda() => (LambdaRef)Value_;
    public StructRef GetStruct() => (StructRef)Value_;
    public IntPtr GetPointer() => (IntPtr)(Value_ ?? IntPtr.Zero);
    public NullRef GetNull()
    {
        Value_ ??= new NullRef();
        return (NullRef)Value_;
    }

    public bool IsNumber() => Type == ValueType.Integer || Type == ValueType.Float;
    public bool IsInteger() => Type == ValueType.Integer;
    public bool IsFloat() => Type == ValueType.Float;
    public bool IsBoolean() => Type == ValueType.Boolean;
    public bool IsString() => Type == ValueType.String;
    public bool IsDate() => Type == ValueType.Date;
    public bool IsList() => Type == ValueType.List;
    public bool IsHashmap() => Type == ValueType.Hashmap;
    public bool IsObject() => Type == ValueType.Object;
    public bool IsLambda() => Type == ValueType.Lambda;
    public bool IsNull() => Type == ValueType.None;
    public bool IsStruct() => Type == ValueType.Struct;
    public bool IsPointer() => Type == ValueType.Pointer;

    public static List<Value> Clone(List<Value> list)
    {
        List<Value> clone = [];

        foreach (var value in list)
        {
            clone.Add(value.Clone());
        }

        return clone;
    }

    public static Dictionary<Value, Value> Clone(Dictionary<Value, Value> hash)
    {
        Dictionary<Value, Value> clone = [];

        foreach (var key in hash.Keys)
        {
            clone.Add(key, hash[key].Clone());
        }

        return clone;
    }

    public Value Clone()
    {
        return Type switch
        {
            ValueType.Integer => CreateInteger(GetInteger()),
            ValueType.Float => CreateFloat(GetFloat()),
            ValueType.Boolean => CreateBoolean(GetBoolean()),
            ValueType.String => CreateString(GetString()),
            ValueType.Date => CreateDate(GetDate()),
            ValueType.List => CreateList(Clone(GetList())),
            ValueType.Hashmap => CreateHashmap(Clone(GetHashmap())),
            ValueType.Object => CreateObject(GetObject()),
            ValueType.Lambda => CreateLambda(GetLambda()),
            ValueType.None => CreateNull(),
            ValueType.Struct => CreateStruct(GetStruct()),
            _ => throw new Exception("Unsupported type for cloning"),
        };
    }

    public void Set(object? value, ValueType type)
    {
        Value_ = value ?? new NullRef();
        Type = type;
    }

    public void Set(Value value, ValueType type)
    {
        Value_ = value.Value_;
        Type = type;
    }

    public void SetValue(Value value)
    {
        Value_ = value.Value_;
        Type = value.Type;
    }

    public void SetValue(long value)
    {
        Value_ = value;
        Type = ValueType.Integer;
    }

    public void SetValue(int value)
    {
        Value_ = (long)value;
        Type = ValueType.Integer;
    }

    public void SetValue(double value)
    {
        Value_ = value;
        Type = ValueType.Float;
    }

    public void SetValue(bool value)
    {
        Value_ = value;
        Type = ValueType.Boolean;
    }

    public void SetValue(string value)
    {
        Value_ = value;
        Type = ValueType.String;
    }

    public void SetValue(DateTime value)
    {
        Value_ = value;
        Type = ValueType.String;
    }

    public void SetValue(List<Value> value)
    {
        Value_ = value;
        Type = ValueType.List;
    }

    public void SetValue(Dictionary<Value, Value> value)
    {
        Value_ = value;
        Type = ValueType.Hashmap;
    }

    public void SetValue(InstanceRef value)
    {
        Value_ = value;
        Type = ValueType.Object;
    }

    public void SetValue(LambdaRef value)
    {
        Value_ = value;
        Type = ValueType.Lambda;
    }

    public void SetValue(StructRef value)
    {
        Value_ = value;
        Type = ValueType.Struct;
    }

    public void SetValue(NullRef value)
    {
        Value_ = value;
        Type = ValueType.None;
    }

    public override bool Equals(object? obj)
    {
        if (obj is not Value other)
        {
            return false;
        }

        if (ReferenceEquals(this, other))
        {
            return true;
        }

        if (Type != other.Type)
        {
            return false;
        }

        // deeper structural check with cycle-protection
        HashSet<(Value, Value)> visitedPairs = [];
        return StructuralEquals(other, visitedPairs);
    }

    private bool StructuralEquals(Value other, HashSet<(Value, Value)> visitedPairs)
    {
        // prevent infinite loops in cyclic structures
        var pair = (this, other);
        if (visitedPairs.Contains(pair))
        {
            return true;
        }

        visitedPairs.Add(pair);

        switch (Type)
        {
            case ValueType.Integer:
                return GetInteger() == other.GetInteger();

            case ValueType.Float:
                return GetFloat().Equals(other.GetFloat());

            case ValueType.Boolean:
                return GetBoolean() == other.GetBoolean();

            case ValueType.String:
                return GetString().Equals(other.GetString());

            case ValueType.Date:
                return GetDate().Equals(other.GetDate());

            case ValueType.List:
                {
                    var thisList = GetList();
                    var otherList = other.GetList();
                    if (thisList.Count != otherList.Count)
                    {
                        return false;
                    }

                    for (int i = 0; i < thisList.Count; i++)
                    {
                        if (!thisList[i].StructuralEquals(otherList[i], visitedPairs))
                        {
                            return false;
                        }
                    }
                    return true;
                }

            case ValueType.Hashmap:
                {
                    var thisMap = GetHashmap();
                    var otherMap = other.GetHashmap();
                    if (thisMap.Count != otherMap.Count)
                    {
                        return false;
                    }

                    foreach (var kvp in thisMap)
                    {
                        var key = kvp.Key;
                        var val = kvp.Value;

                        if (!otherMap.TryGetValue(key, out Value? otherVal))
                        {
                            return false;
                        }

                        if (!val.StructuralEquals(otherVal, visitedPairs))
                        {
                            return false;
                        }
                    }
                    return true;
                }

            case ValueType.Object:
                {
                    var thisObj = GetObject();
                    var otherObj = other.GetObject();

                    if (thisObj.StructName != otherObj.StructName)
                    {
                        return false;
                    }

                    if (thisObj.Identifier != otherObj.Identifier)
                    {
                        return false;
                    }

                    var thisVars = thisObj.InstanceVariables;
                    var otherVars = otherObj.InstanceVariables;
                    if (thisVars.Count != otherVars.Count)
                    {
                        return false;
                    }

                    foreach (var kvp in thisVars)
                    {
                        var key = kvp.Key;
                        var val = kvp.Value;

                        if (!otherVars.TryGetValue(key, out Value? otherVarVal))
                        {
                            return false;
                        }

                        if (!val.StructuralEquals(otherVarVal, visitedPairs))
                        {
                            return false;
                        }
                    }
                    return true;
                }

            case ValueType.Lambda:
                {
                    var thisLambda = GetLambda();
                    var otherLambda = other.GetLambda();
                    return thisLambda.Identifier == otherLambda.Identifier;
                }

            case ValueType.Struct:
                {
                    var thisStruct = GetStruct();
                    var otherStruct = other.GetStruct();
                    return thisStruct.Identifier == otherStruct.Identifier;
                }

            case ValueType.None:
                return true;

            default:
                return false;
        }
    }

    public override int GetHashCode()
    {
        int hash = 17; // prime start
        hash = hash * 31 + (int)Type;

        switch (Type)
        {
            case ValueType.Integer:
                {
                    hash = hash * 31 + GetInteger().GetHashCode();
                    break;
                }
            case ValueType.Float:
                {
                    hash = hash * 31 + GetFloat().GetHashCode();
                    break;
                }
            case ValueType.Boolean:
                {
                    hash = hash * 31 + GetBoolean().GetHashCode();
                    break;
                }
            case ValueType.String:
                {
                    var s = GetString();
                    hash = hash * 31 + (s?.GetHashCode() ?? 0);
                    break;
                }
            case ValueType.Date:
                {
                    var d = GetDate();
                    hash = hash * 31 + d.GetHashCode();
                    break;
                }
            case ValueType.List:
            case ValueType.Hashmap:
            case ValueType.Object:
                {
                    hash = hash * 31 + GetStructuralHashCode();
                    break;
                }
            case ValueType.Lambda:
            case ValueType.Struct:
            case ValueType.None:
            default:
                {
                    break;
                }
        }

        return hash;
    }

    private int GetStructuralHashCode()
    {
        int hash = 17; // prime start
        hash = hash * 31 + (int)Type;

        switch (Type)
        {
            case ValueType.Integer:
            case ValueType.Float:
            case ValueType.Boolean:
            case ValueType.String:
            case ValueType.Date:
                return GetHashCode();

            case ValueType.List:
                {
                    var list = GetList();
                    foreach (var item in list)
                    {
                        hash = hash * 31 + item.GetStructuralHashCode();
                    }
                    break;
                }

            case ValueType.Hashmap:
                {
                    var map = GetHashmap();
                    foreach (var kvp in map)
                    {
                        int keyHash = kvp.Key.GetStructuralHashCode();
                        int valHash = kvp.Value.GetStructuralHashCode();
                        hash = hash * 31 + (keyHash ^ valHash);
                    }
                    break;
                }

            case ValueType.Object:
                {
                    var obj = GetObject();
                    hash = hash * 31 + (obj.StructName?.GetHashCode() ?? 0);
                    hash = hash * 31 + (obj.Identifier?.GetHashCode() ?? 0);

                    foreach (var kvp in obj.InstanceVariables)
                    {
                        int keyHash = kvp.Key.GetHashCode();
                        int valHash = kvp.Value.GetStructuralHashCode();
                        hash = hash * 31 + (keyHash ^ valHash);
                    }
                    break;
                }

            case ValueType.Lambda:
            case ValueType.Struct:
            case ValueType.None:
            default:
                {
                    break;
                }
        }

        return hash;
    }

    private int StructuralHashCode(HashSet<Value> visitedValues)
    {
        if (visitedValues.Contains(this))
        {
            return 0;
        }

        visitedValues.Add(this);

        int hash = 17; // prime start
        hash = hash * 31 + (int)Type;

        switch (Type)
        {
            case ValueType.Integer:
                {
                    hash = hash * 31 + GetInteger().GetHashCode();
                    break;
                }
            case ValueType.Float:
                {
                    hash = hash * 31 + GetFloat().GetHashCode();
                    break;
                }
            case ValueType.Boolean:
                {
                    hash = hash * 31 + GetBoolean().GetHashCode();
                    break;
                }
            case ValueType.String:
                {
                    var s = GetString();
                    hash = hash * 31 + (s?.GetHashCode() ?? 0);
                    break;
                }
            case ValueType.Date:
                {
                    var d = GetDate();
                    hash = hash * 31 + d.GetHashCode();
                    break;
                }
            case ValueType.List:
                {
                    var list = GetList();
                    foreach (var item in list)
                    {
                        hash = hash * 31 + item.StructuralHashCode(visitedValues);
                    }
                    break;
                }
            case ValueType.Hashmap:
                {
                    var map = GetHashmap();
                    foreach (var kvp in map)
                    {
                        int keyHash = kvp.Key.StructuralHashCode(visitedValues);
                        int valHash = kvp.Value.StructuralHashCode(visitedValues);
                        hash = hash * 31 + (keyHash ^ valHash);
                    }
                    break;
                }
            case ValueType.Object:
                {
                    var obj = GetObject();
                    hash = hash * 31 + (obj.StructName?.GetHashCode() ?? 0);
                    hash = hash * 31 + (obj.Identifier?.GetHashCode() ?? 0);

                    foreach (var kvp in obj.InstanceVariables)
                    {
                        int keyHash = kvp.Key.GetHashCode();
                        int valHash = kvp.Value.StructuralHashCode(visitedValues);
                        hash = hash * 31 + (keyHash ^ valHash);
                    }
                    break;
                }
            case ValueType.Lambda:
            case ValueType.Struct:
            case ValueType.None:
            default:
                break;
        }

        return hash;
    }

    /// <summary>
    /// Compare this Value to another object (non-generic IComparable).
    /// </summary>
    /// <param name="obj">The object to compare against.</param>
    /// <returns>
    /// Negative if this is less than obj;
    /// Zero if equal;
    /// Positive if greater.
    /// </returns>
    /// <exception cref="ArgumentException">Thrown if obj is not a Value.</exception>
    public int CompareTo(object? obj)
    {
        if (obj == null)
        {
            return 1; // By convention, any non-null is "greater" than null
        }

        if (obj is not Value otherValue)
        {
            throw new ArgumentException("Object is not a Value", nameof(obj));
        }

        return CompareTo(otherValue);
    }

    /// <summary>
    /// Compare this Value to another Value (generic IComparable).
    /// </summary>
    /// <param name="other">The Value to compare against.</param>
    /// <returns>
    /// Negative if this < other;
    /// Zero if this == other;
    /// Positive if this > other.
    /// </returns>
    public int CompareTo(Value? other)
    {
        if (other is null)
        {
            return 1; // By convention, any non-null is "greater" than null
        }

        // 1) Compare ValueType first
        int typeComparison = Type.CompareTo(other.Type);
        if (typeComparison != 0)
        {
            // If the ValueTypes differ, sort by the numeric ordering of the ValueType enum
            return typeComparison;
        }

        // 2) If the same type, compare the underlying data
        return Type switch
        {
            ValueType.Integer => GetInteger().CompareTo(other.GetInteger()),
            ValueType.Float => GetFloat().CompareTo(other.GetFloat()),// Use GetNumber() or directly compare doubles
            ValueType.Boolean => GetBoolean().CompareTo(other.GetBoolean()),// False < True
            ValueType.String => string.Compare(GetString(), other.GetString(), StringComparison.Ordinal),// Lexical (ordinal) comparison for strings
            ValueType.Date => GetDate().CompareTo(other.GetDate()),// Chronological comparison
            ValueType.List => CompareLists(GetList(), other.GetList()),// Lexicographical comparison of list items
            ValueType.Hashmap => CompareHashmaps(GetHashmap(), other.GetHashmap()),// A simple approach: compare number of entries, then do a structural comparison for tie-breakers
            ValueType.Object => CompareObjects(GetObject(), other.GetObject()),// Compare by StructName, then Identifier, then instance variables for a deeper structural comparison
            ValueType.Lambda => string.Compare(GetLambda().Identifier, other.GetLambda().Identifier, StringComparison.Ordinal),// Compare by the Identifier of the lambda
            ValueType.Struct => string.Compare(GetStruct().Identifier, other.GetStruct().Identifier, StringComparison.Ordinal),// Compare by the Identifier of the struct
            _ => 0,// If both are None, consider them equal
        };
    }

    /// <summary>
    /// Example method for comparing two lists of Value lexicographically.
    /// </summary>
    private int CompareLists(List<Value> xlist, List<Value> ylist)
    {
        int minCount = Math.Min(xlist.Count, ylist.Count);
        for (int i = 0; i < minCount; i++)
        {
            int result = xlist[i].CompareTo(ylist[i]);
            if (result != 0)
            {
                return result;
            }
        }

        // If all elements up to minCount are equal, compare lengths
        return xlist.Count.CompareTo(ylist.Count);
    }

    /// <summary>
    /// Example method for comparing two hashmaps.
    /// This is a simple approach that compares Count first,
    /// and then tries a structural approach based on sorted keys.
    /// You may wish to refine this further depending on your needs.
    /// </summary>
    private int CompareHashmaps(Dictionary<Value, Value> xmap, Dictionary<Value, Value> ymap)
    {
        // Compare counts first
        int countCompare = xmap.Count.CompareTo(ymap.Count);
        if (countCompare != 0)
        {
            return countCompare;
        }

        // Next, do a structural comparison. One approach:
        // 1) Sort the keys from each dictionary
        // 2) Compare the keys pairwise
        // 3) Compare the corresponding values pairwise
        var xkeys = xmap.Keys.ToList();
        var ykeys = ymap.Keys.ToList();

        // Sort the keys using Value's CompareTo
        xkeys.Sort();
        ykeys.Sort();

        for (int i = 0; i < xkeys.Count; i++)
        {
            // Compare keys
            int keyResult = xkeys[i].CompareTo(ykeys[i]);
            if (keyResult != 0)
            {
                return keyResult;
            }

            // Compare values
            int valResult = xmap[xkeys[i]].CompareTo(ymap[ykeys[i]]);
            if (valResult != 0)
            {
                return valResult;
            }
        }

        // If we get here, they're effectively equal
        return 0;
    }

    /// <summary>
    /// Example method for comparing two objects by struct name, identifier, and instance variables.
    /// </summary>
    private int CompareObjects(InstanceRef xobj, InstanceRef yobj)
    {
        // Compare struct names
        int structNameCompare = string.Compare(xobj.StructName, yobj.StructName, StringComparison.Ordinal);
        if (structNameCompare != 0)
        {
            return structNameCompare;
        }

        // Compare identifiers
        int identifierCompare = string.Compare(xobj.Identifier, yobj.Identifier, StringComparison.Ordinal);
        if (identifierCompare != 0)
        {
            return identifierCompare;
        }

        // Compare number of instance variables
        int varCountCompare = xobj.InstanceVariables.Count.CompareTo(yobj.InstanceVariables.Count);
        if (varCountCompare != 0)
        {
            return varCountCompare;
        }

        // If needed, do a deeper structural comparison of instance variables
        // Sort by key name
        var xvars = xobj.InstanceVariables.OrderBy(kvp => kvp.Key).ToList();
        var yvars = yobj.InstanceVariables.OrderBy(kvp => kvp.Key).ToList();

        for (int i = 0; i < xvars.Count; i++)
        {
            // Compare variable names (keys)
            int keyCompare = string.Compare(xvars[i].Key, yvars[i].Key, StringComparison.Ordinal);
            if (keyCompare != 0)
            {
                return keyCompare;
            }

            // Compare variable Values
            int valCompare = xvars[i].Value.CompareTo(yvars[i].Value);
            if (valCompare != 0)
            {
                return valCompare;
            }
        }

        // If identical structurally
        return 0;
    }
}

public enum ValueType
{
    Integer = 0,
    Float = 1,
    Boolean = 2,
    String = 3,
    List = 4,
    Hashmap = 5,
    Object = 6,
    Lambda = 7,
    None = 8,
    Struct = 9,
    Pointer = 10,
    Date = 11,
    Unset = 20,
    Number = 21
};