using System.Collections.Concurrent;
using kiwi.Typing;

namespace kiwi.Runtime;

public class KContext
{
    public HashSet<string> Includes { get; set; } = [];
    public HashSet<string> ImportedPackages { get; set; } = [];
    public Dictionary<string, KPackage> Packages { get; set; } = [];
    public Dictionary<string, KFunction> Functions { get; set; } = [];
    public Dictionary<string, KFunction> Methods { get; set; } = [];
    // ConcurrentDictionary: task threads concurrently register lambdas (with do...end),
    // so this must be thread-safe.
    public ConcurrentDictionary<string, KLambda> Lambdas { get; set; } = new();
    public Dictionary<string, KStruct> Structs { get; set; } = [];
    // ConcurrentDictionary: task threads may map lambda names concurrently.
    public ConcurrentDictionary<string, string> LambdaTable { get; set; } = new();
    public Dictionary<string, Value> Constants { get; set; } = [];
    public Dictionary<string, Value> PackageVariables { get; set; } = [];
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
            cloned.Lambdas[kvp.Key] = kvp.Value.Clone();
        }

        foreach (var kvp in Structs)
        {
            cloned.Structs.Add(kvp.Key, kvp.Value.Clone());
        }

        foreach (var kvp in Constants)
        {
            cloned.Constants.Add(kvp.Key, kvp.Value);
        }

        foreach (var kvp in PackageVariables)
        {
            cloned.PackageVariables.Add(kvp.Key, kvp.Value);
        }

        // Share the same LambdaTable reference — task threads write to it with GUID keys
        // so sharing is safe and avoids stale lookups across interpreter instances.
        cloned.LambdaTable = LambdaTable;
        cloned.ImportedPackages = [.. ImportedPackages];

        return cloned;
    }

    public bool HasConstant(string name) => Constants.ContainsKey(name);
    public bool HasPackageVariable(string name) => PackageVariables.ContainsKey(name);
    public bool HasPackage(string name) => Packages.ContainsKey(name);
    public bool HasFunction(string name) => Functions.ContainsKey(name);
    public bool HasMethod(string name) => Methods.ContainsKey(name);
    public bool HasLambda(string name) => Lambdas.ContainsKey(name);
    public bool HasStruct(string name) => Structs.ContainsKey(name);
    public bool HasMappedLambda(string name) => LambdaTable.ContainsKey(name);
    public void AddMappedLambda(string name, string mappedName) => LambdaTable[name] = mappedName;
}
