namespace Kiwi.Native;

/// <summary>
/// Exports a static method to Kiwi as a function of the extension's package. The source
/// generator converts its arguments and result, and exports <c>kiwi_ext_init</c>, which
/// registers every such method.
/// </summary>
/// <param name="name">
/// The Kiwi name. Leave it out to use the method name in snake_case: <c>SumSquares</c>
/// becomes <c>sum_squares</c>.
/// </param>
[AttributeUsage(AttributeTargets.Method, Inherited = false)]
public sealed class KiwiFunctionAttribute(string? name = null) : Attribute
{
    public string? Name { get; } = name;
}
