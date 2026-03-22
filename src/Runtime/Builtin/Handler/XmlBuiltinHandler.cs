using System.Globalization;
using System.Xml.Linq;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class XmlBuiltinHandler
{
    private enum SchemaKind { String, Integer, Float, Boolean, Null }

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Xml_Parse         => Parse(token, args),
            TokenName.Builtin_Xml_ParseFile     => ParseFile(token, args),
            TokenName.Builtin_Xml_ParseMaps     => ParseMaps(token, args),
            TokenName.Builtin_Xml_ParseFileMaps => ParseFileMaps(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Parse(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, XmlBuiltin.Parse, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, XmlBuiltin.Parse, 0, args[0]);

        try
        {
            var doc = XDocument.Parse(args[0].GetString(), LoadOptions.None);
            return doc.Root is null
                ? Value.CreateNull()
                : ConvertElement(doc.Root);
        }
        catch (System.Xml.XmlException ex)
        {
            throw new KiwiError(token, "XmlError", ex.Message);
        }
    }

    private static Value ParseFile(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, XmlBuiltin.ParseFile, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, XmlBuiltin.ParseFile, 0, args[0]);

        try
        {
            var doc = XDocument.Load(args[0].GetString(), LoadOptions.None);
            return doc.Root is null
                ? Value.CreateNull()
                : ConvertElement(doc.Root);
        }
        catch (System.Xml.XmlException ex)
        {
            throw new KiwiError(token, "XmlError", ex.Message);
        }
        catch (Exception ex)
        {
            throw new KiwiError(token, "XmlError", ex.Message);
        }
    }

    // __xml_parse_maps__(xml_string, tag_or_null, schema_or_null)
    private static Value ParseMaps(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, XmlBuiltin.ParseMaps, 1, 3, args.Count);
        ParameterTypeMismatchError.ExpectString(token, XmlBuiltin.ParseMaps, 0, args[0]);

        try
        {
            var doc = XDocument.Parse(args[0].GetString(), LoadOptions.None);
            if (doc.Root is null)
                return Value.CreateList([]);
            return ExtractRecords(token, doc.Root, args);
        }
        catch (System.Xml.XmlException ex)
        {
            throw new KiwiError(token, "XmlError", ex.Message);
        }
    }

    // __xml_parsefile_maps__(path, tag_or_null, schema_or_null)
    private static Value ParseFileMaps(Token token, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, XmlBuiltin.ParseFileMaps, 1, 3, args.Count);
        ParameterTypeMismatchError.ExpectString(token, XmlBuiltin.ParseFileMaps, 0, args[0]);

        try
        {
            var doc = XDocument.Load(args[0].GetString(), LoadOptions.None);
            if (doc.Root is null)
                return Value.CreateList([]);
            return ExtractRecords(token, doc.Root, args);
        }
        catch (System.Xml.XmlException ex) { throw new KiwiError(token, "XmlError", ex.Message); }
        catch (Exception ex)               { throw new KiwiError(token, "XmlError", ex.Message); }
    }

    private static Value ExtractRecords(Token token, XElement root, List<Value> args)
    {
        var tag    = args.Count > 1 && args[1].IsString()  ? args[1].GetString()    : null;
        var schema = args.Count > 2 && args[2].IsHashmap() ? args[2].GetHashmap()   : null;
        var cache  = schema != null ? BuildXmlSchemaCache(schema) : null;

        var elements = tag != null
            ? FindAllDeep(root, tag).ToList()
            : [root];

        var result = elements.Select(el => ElementToRecord(el, cache)).ToList();
        return Value.CreateList(result);
    }

    // Depth-first search for all elements matching tag name.
    private static IEnumerable<XElement> FindAllDeep(XElement root, string tag)
    {
        foreach (var el in root.Elements())
        {
            if (el.Name.LocalName == tag)
                yield return el;
            foreach (var child in FindAllDeep(el, tag))
                yield return child;
        }
    }

    // Flatten one XElement into a record hashmap:
    //   - attributes → string (or coerced) values by attribute name
    //   - direct child elements → text content by tag name
    private static Value ElementToRecord(XElement el, Dictionary<string, SchemaKind>? cache)
    {
        var record = new Dictionary<Value, Value>();

        foreach (var attr in el.Attributes())
        {
            if (attr.IsNamespaceDeclaration) continue;
            var key = attr.Name.LocalName;
            var val = cache != null && cache.TryGetValue(key, out var kind)
                ? CoerceRaw(attr.Value, kind)
                : Value.CreateString(attr.Value);
            record[Value.CreateString(key)] = val;
        }

        foreach (var child in el.Elements())
        {
            var key = child.Name.LocalName;
            var raw = string.Concat(child.Nodes().OfType<XText>().Select(t => t.Value));
            var val = cache != null && cache.TryGetValue(key, out var kind)
                ? CoerceRaw(raw, kind)
                : Value.CreateString(raw);
            record[Value.CreateString(key)] = val;
        }

        return Value.CreateHashmap(record);
    }

    private static Dictionary<string, SchemaKind> BuildXmlSchemaCache(Dictionary<Value, Value> schema)
    {
        var cache = new Dictionary<string, SchemaKind>(schema.Count, StringComparer.OrdinalIgnoreCase);
        foreach (var kvp in schema)
        {
            if (!kvp.Key.IsString()) continue;
            cache[kvp.Key.GetString()] = (kvp.Value.IsString() ? kvp.Value.GetString() : "string")
                .ToLowerInvariant() switch
            {
                "integer" or "int"              => SchemaKind.Integer,
                "float" or "double" or "number" => SchemaKind.Float,
                "boolean" or "bool"             => SchemaKind.Boolean,
                "null" or "none"                => SchemaKind.Null,
                _                               => SchemaKind.String,
            };
        }
        return cache;
    }

    private static Value CoerceRaw(string raw, SchemaKind kind) => kind switch
    {
        SchemaKind.Integer => raw.Length == 0 ? Value.Default
            : long.TryParse(raw, out var i) ? Value.CreateInteger(i) : Value.Default,
        SchemaKind.Float   => raw.Length == 0 ? Value.Default
            : double.TryParse(raw, NumberStyles.Float, CultureInfo.InvariantCulture, out var d)
                ? Value.CreateFloat(d) : Value.Default,
        SchemaKind.Boolean => Value.CreateBoolean(raw is "true" or "1" or "yes"),
        SchemaKind.String  => Value.CreateString(raw),
        SchemaKind.Null    => Value.Default,
        _                  => Value.CreateString(raw),
    };

    private static Value ConvertElement(XElement el)
    {
        var attrs    = new Dictionary<Value, Value>();
        var children = new List<Value>();

        foreach (var attr in el.Attributes())
        {
            if (!attr.IsNamespaceDeclaration)
            {
                attrs[Value.CreateString(attr.Name.LocalName)] = Value.CreateString(attr.Value);
            }
        }

        foreach (var node in el.Nodes())
        {
            var child = ConvertNode(node);
            if (child is not null)
            {
                children.Add(child);
            }
        }

        return MakeNode(el.Name.LocalName, attrs, children, string.Empty);
    }

    private static Value? ConvertNode(XNode node) => node switch
    {
        XElement   el   => ConvertElement(el),
        XCData     cd   => MakeNode("#cdata",   [], [], cd.Value),
        XText      text => string.IsNullOrWhiteSpace(text.Value) ? null
                            : MakeNode("#text", [], children: [], text.Value),
        XComment   cmt  => MakeNode("#comment", [], [], cmt.Value),
        XProcessingInstruction pi =>
            MakeNode("#pi",
                new Dictionary<Value, Value> { [Value.CreateString("name")] = Value.CreateString(pi.Target) },
                [], pi.Data),
        _ => null,
    };

    private static Value MakeNode(string tag, Dictionary<Value, Value> attrs, List<Value> children, string content)
    {
        var map = new Dictionary<Value, Value>
        {
            [Value.CreateString("tag")]      = Value.CreateString(tag),
            [Value.CreateString("attrs")]    = Value.CreateHashmap(attrs),
            [Value.CreateString("children")] = Value.CreateList(children),
            [Value.CreateString("content")]  = Value.CreateString(content),
        };
        return Value.CreateHashmap(map);
    }
}
