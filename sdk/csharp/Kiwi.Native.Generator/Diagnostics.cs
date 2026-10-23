using Microsoft.CodeAnalysis;

namespace Kiwi.Native.Generator;

internal static class Diagnostics
{
    private const string Category = "Kiwi.Native";

    private static DiagnosticDescriptor Error(string id, string title, string message) =>
        new(id, title, message, Category, DiagnosticSeverity.Error, isEnabledByDefault: true);

    public static readonly DiagnosticDescriptor UnsupportedParameter = Error("KIWI001",
        "Unsupported parameter type",
        "Parameter '{0}' of {1} has type {2}, which can't be converted from a Kiwi value. Use a supported type, or KiwiValue to read it yourself.");

    public static readonly DiagnosticDescriptor UnsupportedReturn = Error("KIWI002",
        "Unsupported return type",
        "{0} returns {1}, which can't be converted to a Kiwi value. Return a supported type, or a KiwiValue.");

    public static readonly DiagnosticDescriptor NotStatic = Error("KIWI003",
        "Kiwi functions must be static",
        "{0} must be static to be a Kiwi function");

    public static readonly DiagnosticDescriptor NotAccessible = Error("KIWI004",
        "Kiwi functions must be accessible",
        "{0} and the types containing it must be public or internal, so the generated export can call it");

    public static readonly DiagnosticDescriptor Generic = Error("KIWI005",
        "Kiwi functions can't be generic",
        "{0} can't be a Kiwi function because it is generic, or in a generic type");

    public static readonly DiagnosticDescriptor ByRef = Error("KIWI006",
        "Kiwi functions can't take ref, out or in parameters",
        "Parameter '{0}' of {1} is passed by reference; Kiwi passes arguments by value");

    public static readonly DiagnosticDescriptor ContextNotFirst = Error("KIWI007",
        "KiwiContext must be the first parameter",
        "Parameter '{0}' of {1} is a KiwiContext; a KiwiContext can only be the first parameter");

    public static readonly DiagnosticDescriptor DuplicateName = Error("KIWI008",
        "Duplicate Kiwi function name",
        "{0} and {1} are both exported as '{2}'");

    public static readonly DiagnosticDescriptor InvalidName = Error("KIWI009",
        "Invalid Kiwi function name",
        "'{0}' isn't a valid Kiwi function name: use letters, digits and underscores, not starting with a digit");

    public static readonly DiagnosticDescriptor UnsafeRequired = Error("KIWI010",
        "Kiwi extensions need unsafe code",
        "The generated kiwi_ext_init export uses function pointers. Set <AllowUnsafeBlocks>true</AllowUnsafeBlocks> in the project, or reference the Kiwi.Native package, which sets it.");

    public static readonly DiagnosticDescriptor UnsupportedDefault = Error("KIWI011",
        "Unsupported default value",
        "The default value of parameter '{0}' of {1} can't be used by a Kiwi function; use a constant number, boolean, string or null");
}
