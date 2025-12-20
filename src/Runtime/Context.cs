using kiwi.Typing;

namespace kiwi.Runtime;

public class KContext
{
    public HashSet<string> Includes { get; set; } = [];
    public Dictionary<string, KPackage> Packages { get; set; } = [];
    public Dictionary<string, KFunction> Functions { get; set; } = [];
    public Dictionary<string, KFunction> Methods { get; set; } = [];
    public Dictionary<string, KLambda> Lambdas { get; set; } = [];
    public Dictionary<string, KStruct> Structs { get; set; } = [];
    public Dictionary<string, string> LambdaTable { get; set; } = [];
    public Dictionary<string, Value> Constants { get; set; } = [];
    public EventBus Events { get; } = new();

    public KContext Clone()
    {
        KContext cloned = new ();

        foreach (var kvp in Packages)
        {
            cloned.Packages.Add(kvp.Key, kvp.Value.Clone());
        }

        foreach (var kvp in Functions)
        {
            cloned.Functions.Add(kvp.Key, kvp.Value.Clone());
        }

        foreach (var kvp in Methods)
        {
            cloned.Methods.Add(kvp.Key, kvp.Value.Clone());
        }

        foreach (var kvp in Lambdas)
        {
            cloned.Lambdas.Add(kvp.Key, kvp.Value.Clone());
        }

        foreach (var kvp in Structs)
        {
            cloned.Structs.Add(kvp.Key, kvp.Value.Clone());
        }

        foreach (var kvp in Constants)
        {
            cloned.Constants.Add(kvp.Key, kvp.Value);
        }

        cloned.LambdaTable = LambdaTable;

        return cloned;
    }

    public bool HasConstant(string name) => Constants.ContainsKey(name);
    public bool HasPackage(string name) => Packages.ContainsKey(name);
    public bool HasFunction(string name) => Functions.ContainsKey(name);
    public bool HasMethod(string name) => Methods.ContainsKey(name);
    public bool HasLambda(string name) => Lambdas.ContainsKey(name);
    public bool HasStruct(string name) => Structs.ContainsKey(name);
    public bool HasMappedLambda(string name) => LambdaTable.ContainsKey(name);
    public void AddMappedLambda(string name, string mappedName) => LambdaTable[name] = mappedName;
}