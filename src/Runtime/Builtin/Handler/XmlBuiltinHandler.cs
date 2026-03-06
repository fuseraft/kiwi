using System.Xml.Linq;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class XmlBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Xml_Parse     => Parse(token, args),
            TokenName.Builtin_Xml_ParseFile => ParseFile(token, args),
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
