using System.Globalization;
using Microsoft.CodeAnalysis;
using Microsoft.CodeAnalysis.CSharp;

namespace Kiwi.Native.Generator;

/// <summary>
/// Writes the C# expressions that convert between Kiwi values and .NET types. Readers and
/// writers return null for a type they can't convert.
/// </summary>
internal static class Marshalling
{
    public const string Ctx = "global::Kiwi.Native.KiwiContext";
    public const string Value = "global::Kiwi.Native.KiwiValue";

    private static readonly SymbolDisplayFormat Format = SymbolDisplayFormat.FullyQualifiedFormat
        .AddMiscellaneousOptions(SymbolDisplayMiscellaneousOptions.IncludeNullableReferenceTypeModifier);

    public static string Display(ITypeSymbol type) => type.ToDisplayString(Format);

    public static bool IsKiwiContext(ITypeSymbol type) => IsNamed(type, "Kiwi.Native.KiwiContext");

    private static bool IsKiwiValue(ITypeSymbol type) => IsNamed(type, "Kiwi.Native.KiwiValue");

    private static bool IsNamed(ITypeSymbol type, string fullName) =>
        type.OriginalDefinition.ToDisplayString() == fullName;

    private static bool IsByteArray(ITypeSymbol type) =>
        type is IArrayTypeSymbol { Rank: 1, ElementType.SpecialType: SpecialType.System_Byte };

    /// <summary>An expression reading <paramref name="value"/> as <paramref name="type"/>.</summary>
    public static string? Read(ITypeSymbol type, string ctx, string value, int depth)
    {
        // A null Kiwi value is allowed only where the .NET type can hold null.
        if (type is INamedTypeSymbol { OriginalDefinition.SpecialType: SpecialType.System_Nullable_T } nullable)
        {
            var inner = Read(nullable.TypeArguments[0], ctx, value, depth);
            return inner == null ? null : $"({ctx}.IsNone({value}) ? ({Display(type)})null : {inner})";
        }

        if (type.IsReferenceType && type.NullableAnnotation == NullableAnnotation.Annotated)
        {
            var inner = ReadNonNull(type.WithNullableAnnotation(NullableAnnotation.NotAnnotated), ctx, value, depth);
            return inner == null ? null : $"({ctx}.IsNone({value}) ? ({Display(type)})null : {inner})";
        }

        return ReadNonNull(type, ctx, value, depth);
    }

    private static string? ReadNonNull(ITypeSymbol type, string ctx, string value, int depth)
    {
        if (IsKiwiValue(type)) return value;
        if (IsByteArray(type)) return $"{ctx}.GetBytes({value})";

        switch (type.SpecialType)
        {
            case SpecialType.System_Int64:   return $"{ctx}.GetInt64({value})";
            case SpecialType.System_Int32:   return $"{ctx}.GetInt32({value})";
            case SpecialType.System_Double:  return $"{ctx}.GetDouble({value})";
            case SpecialType.System_Single:  return $"{ctx}.GetSingle({value})";
            case SpecialType.System_Boolean: return $"{ctx}.GetBoolean({value})";
            case SpecialType.System_String:  return $"{ctx}.GetString({value})";
            case SpecialType.System_IntPtr:  return $"{ctx}.GetPointer({value})";
            case SpecialType.System_Object:  return $"global::Kiwi.Native.KiwiRuntime.NotNull({ctx}.ToObject({value}))";
        }

        if (type is IArrayTypeSymbol { Rank: 1 } array)
        {
            var item = ItemReader(array.ElementType, depth);
            return item == null ? null : $"{ctx}.GetArray({value}, {item})";
        }

        if (type is INamedTypeSymbol { IsGenericType: true } named)
        {
            var definition = named.OriginalDefinition.ToDisplayString();
            switch (definition)
            {
                case "System.Collections.Generic.List<T>":
                case "System.Collections.Generic.IList<T>":
                case "System.Collections.Generic.ICollection<T>":
                case "System.Collections.Generic.IEnumerable<T>":
                case "System.Collections.Generic.IReadOnlyList<T>":
                case "System.Collections.Generic.IReadOnlyCollection<T>":
                {
                    var item = ItemReader(named.TypeArguments[0], depth);
                    return item == null ? null : $"{ctx}.GetList({value}, {item})";
                }
                case "System.Collections.Generic.Dictionary<TKey, TValue>":
                case "System.Collections.Generic.IDictionary<TKey, TValue>":
                case "System.Collections.Generic.IReadOnlyDictionary<TKey, TValue>":
                {
                    var keyType = named.TypeArguments[0];
                    if (keyType.NullableAnnotation == NullableAnnotation.Annotated || keyType is INamedTypeSymbol { OriginalDefinition.SpecialType: SpecialType.System_Nullable_T })
                        return null;
                    var key = ItemReader(keyType, depth);
                    var item = ItemReader(named.TypeArguments[1], depth);
                    return key == null || item == null ? null : $"{ctx}.GetDictionary({value}, {key}, {item})";
                }
            }
        }

        return null;
    }

    private static string? ItemReader(ITypeSymbol type, int depth)
    {
        var (c, v) = ($"__c{depth}", $"__v{depth}");
        var body = Read(type, c, v, depth + 1);
        return body == null ? null : $"static {Display(type)} ({Ctx} {c}, {Value} {v}) => {body}";
    }

    /// <summary>An expression converting <paramref name="expr"/> of <paramref name="type"/> to a KiwiValue.</summary>
    public static string? Write(ITypeSymbol type, string ctx, string expr, int depth)
    {
        if (type is INamedTypeSymbol { OriginalDefinition.SpecialType: SpecialType.System_Nullable_T } nullable)
        {
            var name = $"__n{depth}";
            var inner = Write(nullable.TypeArguments[0], ctx, name, depth + 1);
            return inner == null ? null : $"({expr} is {Display(nullable.TypeArguments[0])} {name} ? {inner} : {ctx}.None())";
        }

        if (IsKiwiValue(type)) return expr;
        if (IsByteArray(type)) return $"{ctx}.Bytes({expr})";

        switch (type.SpecialType)
        {
            case SpecialType.System_Int64:
            case SpecialType.System_Int32:
            case SpecialType.System_Int16:
            case SpecialType.System_SByte:
            case SpecialType.System_Byte:
            case SpecialType.System_UInt16:
            case SpecialType.System_UInt32:
                return $"{ctx}.Int({expr})";
            case SpecialType.System_Double:
            case SpecialType.System_Single:
                return $"{ctx}.Float({expr})";
            case SpecialType.System_Boolean: return $"{ctx}.Bool({expr})";
            case SpecialType.System_String:  return $"{ctx}.String({expr})";
            case SpecialType.System_IntPtr:  return $"{ctx}.Pointer({expr})";
            case SpecialType.System_UInt64:
            case SpecialType.System_Decimal:
            case SpecialType.System_Char:
            case SpecialType.System_Object:
                return $"{ctx}.From({expr})";
        }

        // A dictionary is also a sequence of pairs, so it is matched first.
        var pairs = SequenceOf(type, "System.Collections.Generic.KeyValuePair<TKey, TValue>");
        if (pairs != null)
        {
            var key = ItemWriter(pairs.TypeArguments[0], depth);
            var item = ItemWriter(pairs.TypeArguments[1], depth);
            return key == null || item == null ? null : $"{ctx}.NewHashmap({expr}, {key}, {item})";
        }

        var sequence = SequenceItem(type);
        if (sequence != null)
        {
            var item = ItemWriter(sequence, depth);
            return item == null ? null : $"{ctx}.NewList({expr}, {item})";
        }

        return null;
    }

    private static string? ItemWriter(ITypeSymbol type, int depth)
    {
        var (c, x) = ($"__c{depth}", $"__x{depth}");
        var body = Write(type, c, x, depth + 1);
        return body == null ? null : $"static {Value} ({Ctx} {c}, {Display(type)} {x}) => {body}";
    }

    // The T of the IEnumerable<T> a type implements, if T is a constructed `definition`.
    private static INamedTypeSymbol? SequenceOf(ITypeSymbol type, string definition) =>
        SequenceItem(type) is INamedTypeSymbol item && item.OriginalDefinition.ToDisplayString() == definition ? item : null;

    private static ITypeSymbol? SequenceItem(ITypeSymbol type)
    {
        if (type.SpecialType == SpecialType.System_String) return null;
        if (type is IArrayTypeSymbol { Rank: 1 } array) return array.ElementType;

        if (type is INamedTypeSymbol named && named.OriginalDefinition.SpecialType == SpecialType.System_Collections_Generic_IEnumerable_T)
            return named.TypeArguments[0];

        foreach (var iface in type.AllInterfaces)
        {
            if (iface.OriginalDefinition.SpecialType == SpecialType.System_Collections_Generic_IEnumerable_T)
                return iface.TypeArguments[0];
        }
        return null;
    }

    /// <summary>A C# literal for a parameter's default value, or null if it has none we can write.</summary>
    public static string? DefaultLiteral(IParameterSymbol parameter)
    {
        var value = parameter.ExplicitDefaultValue;
        var type = parameter.Type is INamedTypeSymbol { OriginalDefinition.SpecialType: SpecialType.System_Nullable_T } n
            ? n.TypeArguments[0]
            : parameter.Type;

        return value switch
        {
            null     => $"({Display(parameter.Type)})default",
            bool b   => b ? "true" : "false",
            string s => SymbolDisplay.FormatLiteral(s, quote: true),
            int i    => Cast(type, i.ToString(CultureInfo.InvariantCulture)),
            long l   => Cast(type, l.ToString(CultureInfo.InvariantCulture) + "L"),
            double d => Cast(type, Double(d)),
            float f  => Cast(type, float.IsNaN(f) || float.IsInfinity(f) ? Double(f) : f.ToString("R", CultureInfo.InvariantCulture) + "f"),
            _        => null,
        };
    }

    private static string Cast(ITypeSymbol type, string literal) => $"({Display(type)})({literal})";

    private static string Double(double d) =>
        double.IsNaN(d) ? "double.NaN"
        : double.IsPositiveInfinity(d) ? "double.PositiveInfinity"
        : double.IsNegativeInfinity(d) ? "double.NegativeInfinity"
        : d.ToString("R", CultureInfo.InvariantCulture) + "d";
}
