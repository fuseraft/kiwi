using System.Collections.Concurrent;
using kiwi.Typing;

namespace kiwi.Runtime;

/// <summary>
/// Runtime state shared by every VM in the process, including task VMs on other threads.
/// The registries are concurrent dictionaries: reads (every global lookup, every method
/// call) take no lock, and tasks may define functions, structs and packages at any time.
/// </summary>
public class KContext
{
    // Sets of file paths / package names; the byte value is unused.
    public ConcurrentDictionary<string, byte> Includes { get; set; } = new();
    public ConcurrentDictionary<string, byte> ImportedPackages { get; set; } = new();
    public ConcurrentDictionary<string, KPackage> Packages { get; set; } = new();
    public ConcurrentDictionary<string, KFunction> Functions { get; set; } = new();
    public ConcurrentDictionary<string, KFunction> Methods { get; set; } = new();
    public ConcurrentDictionary<string, KLambda> Lambdas { get; set; } = new();
    public ConcurrentDictionary<string, KStruct> Structs { get; set; } = new();
    public ConcurrentDictionary<string, string> LambdaTable { get; set; } = new();
    public ConcurrentDictionary<string, Value> Constants { get; set; } = new();
    public ConcurrentDictionary<string, Value> PackageVariables { get; set; } = new();
    public EventBus Events { get; } = new();

    /// <summary>
    /// Held while a package is imported or a file included, so a second task importing the
    /// same package waits for the first to finish instead of seeing it half-defined.
    /// Reentrant: an import may import other packages.
    /// </summary>
    public object ImportLock { get; } = new();

    public KContext Clone()
    {
        KContext cloned = new ();

        foreach (var kvp in Packages)
        {
            cloned.Packages[kvp.Key] = kvp.Value.Clone();
        }

        foreach (var kvp in Functions)
        {
            cloned.Functions[kvp.Key] = kvp.Value.Clone();
        }

        foreach (var kvp in Methods)
        {
            cloned.Methods[kvp.Key] = kvp.Value.Clone();
        }

        foreach (var kvp in Lambdas)
        {
            cloned.Lambdas[kvp.Key] = kvp.Value.Clone();
        }

        foreach (var kvp in Structs)
        {
            cloned.Structs[kvp.Key] = kvp.Value.Clone();
        }

        foreach (var kvp in Constants)
        {
            cloned.Constants[kvp.Key] = kvp.Value;
        }

        foreach (var kvp in PackageVariables)
        {
            cloned.PackageVariables[kvp.Key] = kvp.Value;
        }

        // Share the same LambdaTable reference — task threads write to it with GUID keys
        // so sharing is safe and avoids stale lookups across interpreter instances.
        cloned.LambdaTable = LambdaTable;
        cloned.ImportedPackages = new(ImportedPackages);

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
