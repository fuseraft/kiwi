using kiwi.Runtime.Builtin.Operation;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;
using kiwi.Runtime.Builtin.Util;

namespace kiwi.Runtime.Builtin.Handler;

public static class CoreBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, Value value, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Core_Chars => Chars(token, value, args),
            TokenName.Builtin_Core_Chomp => Chomp(token, value, args),
            TokenName.Builtin_Core_Join => Join(token, value, args),
            TokenName.Builtin_Core_HasKey => HasKey(token, value, args),
            TokenName.Builtin_Core_Size => Size(token, value, args),
            TokenName.Builtin_Core_Uppercase => Uppercase(token, value, args),
            TokenName.Builtin_Core_Lowercase => Lowercase(token, value, args),
            TokenName.Builtin_Core_BeginsWith => BeginsWith(token, value, args),
            TokenName.Builtin_Core_Contains => Contains(token, value, args),
            TokenName.Builtin_Core_EndsWith => EndsWith(token, value, args),
            TokenName.Builtin_Core_LeftTrim => LeftTrim(token, value, args),
            TokenName.Builtin_Core_RightTrim => RightTrim(token, value, args),
            TokenName.Builtin_Core_Trim => Trim(token, value, args),
            TokenName.Builtin_Core_Empty => Empty(token, value, args),
            TokenName.Builtin_Core_Type => Type(token, value, args),
            TokenName.Builtin_Core_First => First(token, value, args),
            TokenName.Builtin_Core_Last => Last(token, value, args),
            TokenName.Builtin_Core_Append => Push(token, value, args),
            TokenName.Builtin_Core_Push => Push(token, value, args),
            TokenName.Builtin_Core_Pop => Pop(token, value, args),
            TokenName.Builtin_Core_Clear => Clear(token, value, args),
            TokenName.Builtin_Core_Replace => Replace(token, value, args),
            TokenName.Builtin_Core_Remove => Remove(token, value, args),
            TokenName.Builtin_Core_RemoveAt => RemoveAt(token, value, args),
            TokenName.Builtin_Core_Get => Get(token, value, args),
            TokenName.Builtin_Core_Set => Set(token, value, args),
            TokenName.Builtin_Core_Truthy => Truthy(token, value, args),
            TokenName.Builtin_Core_Split => Split(token, value, args),
            TokenName.Builtin_Core_Lines => Lines(token, value, args),
            TokenName.Builtin_Core_Clone => Clone(token, value, args),
            TokenName.Builtin_Core_Reverse => Reverse(token, value, args),
            TokenName.Builtin_Core_IndexOf => IndexOf(token, value, args),
            TokenName.Builtin_Core_LastIndexOf => LastIndexOf(token, value, args),
            TokenName.Builtin_Core_Substring => Substring(token, value, args),
            TokenName.Builtin_Core_Concat => Concat(token, value, args),
            TokenName.Builtin_Core_Unique => Unique(token, value, args),
            TokenName.Builtin_Core_Count => Count(token, value, args),
            TokenName.Builtin_Core_Enqueue => Enqueue(token, value, args),
            TokenName.Builtin_Core_Dequeue => Dequeue(token, value, args),
            TokenName.Builtin_Core_Shift => Shift(token, value, args),
            TokenName.Builtin_Core_Unshift => Unshift(token, value, args),
            TokenName.Builtin_Core_Insert => Insert(token, value, args),
            TokenName.Builtin_Core_Keys => Keys(token, value, args),
            TokenName.Builtin_Core_Values => Values(token, value, args),
            TokenName.Builtin_Core_Zip => Zip(token, value, args),
            TokenName.Builtin_Core_Flatten => Flatten(token, value, args),
            TokenName.Builtin_Core_Merge => Merge(token, value, args),
            TokenName.Builtin_Core_Rotate => Rotate(token, value, args),
            TokenName.Builtin_Core_Slice => Slice(token, value, args),
            TokenName.Builtin_Core_IsA => IsA(token, value, args),
            TokenName.Builtin_Core_ToBytes => ToBytes(token, value, args),
            TokenName.Builtin_Core_ToHex => ToHex(token, value, args),
            TokenName.Builtin_Core_ToFloat => ToFloat(token, value, args),
            TokenName.Builtin_Core_ToInteger => ToInteger(token, value, args),
            TokenName.Builtin_Core_ToString => ToString(token, value, args),
            TokenName.Builtin_Core_ToDate => ToDate(token, value, args),
            TokenName.Builtin_Core_ToList => ToList(token, value, args),
            TokenName.Builtin_Core_Swap => Swap(token, value, args),
            TokenName.Builtin_Core_Pretty => Pretty(token, value, args),
            TokenName.Builtin_Core_Hour => Hour(token, value, args),
            TokenName.Builtin_Core_Minute => Minute(token, value, args),
            TokenName.Builtin_Core_Second => Second(token, value, args),
            TokenName.Builtin_Core_Millisecond => Millisecond(token, value, args),
            TokenName.Builtin_Core_Day => Day(token, value, args),
            TokenName.Builtin_Core_Month => Month(token, value, args),
            TokenName.Builtin_Core_Year => Year(token, value, args),
            TokenName.Builtin_Core_Between => Between(token, value, args),
            TokenName.Builtin_Core_RReplace => RReplace(token, value, args),
            TokenName.Builtin_Core_RSplit => RSplit(token, value, args),
            TokenName.Builtin_Core_Find => Find(token, value, args),
            TokenName.Builtin_Core_Match => Match(token, value, args),
            TokenName.Builtin_Core_Matches => Matches(token, value, args),
            TokenName.Builtin_Core_MatchesAll => MatchesAll(token, value, args),
            TokenName.Builtin_Core_Scan => Scan(token, value, args),
            /*
            TokenName.Builtin_Core_Tokens => Tokens(token, value, args),
            */
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Match(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Match, 1, args.Count);

        TypeError.ExpectString(token, value);
        TypeError.ExpectString(token, args[0]);

        var stringValue = value.GetString();
        var pattern = args[0].GetString();

        return Value.CreateList(RegexUtil.Match(stringValue, pattern));
    }

    private static Value Matches(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Matches, 1, args.Count);

        TypeError.ExpectString(token, value);
        TypeError.ExpectString(token, args[0]);

        var stringValue = value.GetString();
        var pattern = args[0].GetString();

        return Value.CreateBoolean(RegexUtil.Matches(stringValue, pattern));
    }

    private static Value MatchesAll(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.MatchesAll, 1, args.Count);

        TypeError.ExpectString(token, value);
        TypeError.ExpectString(token, args[0]);

        var stringValue = value.GetString();
        var pattern = args[0].GetString();

        return Value.CreateBoolean(RegexUtil.MatchesAll(stringValue, pattern));
    }

    private static Value Find(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Find, 1, args.Count);

        TypeError.ExpectString(token, value);
        TypeError.ExpectString(token, args[0]);

        var stringValue = value.GetString();
        var pattern = args[0].GetString();

        return Value.CreateString(RegexUtil.Find(stringValue, pattern));
    }

    private static Value Scan(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Scan, 1, args.Count);

        TypeError.ExpectString(token, value);
        TypeError.ExpectString(token, args[0]);

        var stringValue = value.GetString();
        var pattern = args[0].GetString();

        return Value.CreateList(RegexUtil.Scan(stringValue, pattern));
    }

    private static Value RSplit(Token token, Value value, List<Value> args)
    {
        if (args.Count != 1 && args.Count != 2)
        {
            throw new ParameterCountMismatchError(token, CoreBuiltin.RSplit);
        }

        TypeError.ExpectString(token, value);
        TypeError.ExpectString(token, args[0]);

        var input = value.GetString();
        var delimiter = args[0].GetString();
        List<Value> newList = [];
        long limit = -1;

        if (args.Count == 2)
        {
            TypeError.ExpectInteger(token, args[1]);
            limit = args[1].GetInteger();
        }

        if (string.IsNullOrEmpty(delimiter))
        {
            foreach (var c in input)
            {
                newList.Add(Value.CreateString(c));
            }
        }
        else
        {
            var tokens = RegexUtil.RSplit(input, delimiter, limit);
            foreach (var t in tokens)
            {
                newList.Add(t);
            }
        }

        return Value.CreateList(newList);
    }

    private static Value RReplace(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.RReplace, 2, args.Count);

        TypeError.ExpectString(token, value);
        TypeError.ExpectString(token, args[0]);
        TypeError.ExpectString(token, args[1]);

        return Value.CreateString(RegexUtil.RReplace(value.GetString(), args[0].GetString(), args[1].GetString()));
    }

    private static Value Between(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Between, 2, args.Count);

        if (value.IsNumber())
        {
            ParameterTypeMismatchError.ExpectNumber(token, CoreBuiltin.Between, 0, args[0]);
            ParameterTypeMismatchError.ExpectNumber(token, CoreBuiltin.Between, 1, args[1]);

            List<double> range = [args[0].GetNumber(), args[1].GetNumber()];
            var test = value.GetNumber();

            return Value.CreateBoolean(range.Min() <= test && test <= range.Max());
        }
        else if (value.IsString())
        {
            ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.Between, 0, args[0]);
            ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.Between, 1, args[1]);

            List<string> range = [args[0].GetString(), args[1].GetString()];
            var test = value.GetString();

            return Value.CreateBoolean(range.Min()!.CompareTo(test) <= 0 && test.CompareTo(range.Max()!) <= 0);
        }
        else if (value.IsDate())
        {
            ParameterTypeMismatchError.ExpectDate(token, CoreBuiltin.Between, 0, args[0]);
            ParameterTypeMismatchError.ExpectDate(token, CoreBuiltin.Between, 1, args[1]);

            List<DateTime> range = [args[0].GetDate(), args[1].GetDate()];
            var test = value.GetDate();

            return Value.CreateBoolean(range.Min() <= test && test <= range.Max());
        }

        throw new InvalidOperationError(token, $"Invalid type for `{CoreBuiltin.Between}`: {TypeRegistry.GetTypeName(value)}");
    }

    private static Value Hour(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Hour, 0, args.Count);

        TypeError.ExpectDate(token, value);

        return Value.CreateInteger(value.GetDate().Hour);
    }

    private static Value Minute(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Minute, 0, args.Count);

        TypeError.ExpectDate(token, value);

        return Value.CreateInteger(value.GetDate().Minute);
    }

    private static Value Second(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Second, 0, args.Count);

        TypeError.ExpectDate(token, value);

        return Value.CreateInteger(value.GetDate().Second);
    }

    private static Value Millisecond(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Millisecond, 0, args.Count);

        TypeError.ExpectDate(token, value);

        return Value.CreateInteger(value.GetDate().Millisecond);
    }

    private static Value Day(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Day, 0, args.Count);

        TypeError.ExpectDate(token, value);

        return Value.CreateInteger(value.GetDate().Day);
    }

    private static Value Month(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Month, 0, args.Count);

        TypeError.ExpectDate(token, value);

        return Value.CreateInteger(value.GetDate().Month);
    }

    private static Value Year(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Year, 0, args.Count);

        TypeError.ExpectDate(token, value);

        return Value.CreateInteger(value.GetDate().Year);
    }

    private static Value Pretty(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Pretty, 0, args.Count);

        var pretty = Serializer.PrettySerialize(value);
        return Value.CreateString(pretty);
    }

    private static Value IsA(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.IsA, 1, args.Count);

        ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.IsA, 0, args[0]);
        
        var typeName = args[0].GetString();

        return value.Type switch
        {
            Typing.ValueType.Integer => Value.CreateBoolean(typeName.Equals("integer")),
            Typing.ValueType.Float => Value.CreateBoolean(typeName.Equals("float")),
            Typing.ValueType.Boolean => Value.CreateBoolean(typeName.Equals("boolean")),
            Typing.ValueType.String => Value.CreateBoolean(typeName.Equals("string")),
            Typing.ValueType.Date => Value.CreateBoolean(typeName.Equals("date")),
            Typing.ValueType.List => Value.CreateBoolean(typeName.Equals("list")),
            Typing.ValueType.Hashmap => Value.CreateBoolean(typeName.Equals("hashmap")),
            Typing.ValueType.Object => Value.CreateBoolean(typeName.Equals("object")),
            Typing.ValueType.Lambda => Value.CreateBoolean(typeName.Equals("lambda")),
            Typing.ValueType.Bytes => Value.CreateBoolean(typeName.Equals("bytes")),
            Typing.ValueType.None => Value.CreateBoolean(typeName.Equals("none")),
            _ => Value.False,
        };
    }

    private static Value ToBytes(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.ToBytes, 0, args.Count);

        if (value.IsString())
        {
            var stringValue = value.GetString();
            var bytes = System.Text.Encoding.UTF8.GetBytes(stringValue);
            var byteList = new List<byte>(bytes.Length);

            foreach (var b in bytes)
            {
                byteList.Add(b);
            }

            return Value.CreateBytes([.. byteList]);
        }
        else if (value.IsList())
        {
            var listElements = value.GetList();
            var resultBytes = new List<byte>();

            foreach (var item in listElements)
            {
                if (!item.IsString())
                {
                    throw new InvalidOperationError(token, "Expected a list of strings for byte conversion.");
                }

                var stringValue = item.GetString();
                var bytes = System.Text.Encoding.UTF8.GetBytes(stringValue);

                foreach (var b in bytes)
                {
                    resultBytes.Add(b);
                }
            }

            return Value.CreateBytes([.. resultBytes]);
        }
        else if (value.IsBytes())
        {
            return Value.CreateBytes(value.GetBytes());
        }

        throw new InvalidOperationError(token, "Expected a string or list of strings to convert to bytes.");
    }

    private static Value ToHex(Token token, Value value, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, CoreBuiltin.ToHex);
        }

        // value is an integer
        if (value.IsInteger())
        {
            var number = value.GetInteger();
            var hexString = number.ToString("x");
            return Value.CreateString(hexString);
        }

        // if not an integer, we expect bytes
        TypeError.ExpectBytes(token, value);

        var elements = value.GetBytes();
        if (elements.Length == 0)
        {
            throw new InvalidOperationError(token, "Cannot convert empty bytes to hex.");
        }

        var width = 2;

        if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectNumber(token, CoreBuiltin.ToI, 0, args[0]);

            width = (int)args[0].GetNumber();
            if (width < 2)
            {
                throw new InvalidOperationError(token, "Width must be >= 2.");
            }
        }

        var sb = new System.Text.StringBuilder();

        foreach (var byteValue in elements)
        {
            var b = (byte)(byteValue & 0xFF);
            sb.Append(b.ToString("x").PadLeft(width, '0'));
        }

        return Value.CreateString(sb.ToString());
    }

    private static Value ToFloat(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.ToF, 0, args.Count);

        if (value.IsString())
        {
            var stringValue = value.GetString();

            if (double.TryParse(stringValue, System.Globalization.NumberStyles.Float | System.Globalization.NumberStyles.AllowThousands, System.Globalization.CultureInfo.InvariantCulture, out double f))
            {
                return Value.CreateFloat(f);
            }
            else
            {
                throw new ConversionError(token, $"Cannot convert non-numeric value to a float: `{stringValue}`");
            }
        }
        else if (value.IsInteger())
        {
            return Value.CreateFloat(value.GetInteger());
        }
        else if (value.IsFloat())
        {
            return value;
        }

        throw new ConversionError(token, "Cannot convert non-numeric value to a float.");
    }

    private static Value ToInteger(Token token, Value value, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, CoreBuiltin.ToI);
        }

        var numberBase = 10;

        // validate base if detected
        if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectNumber(token, CoreBuiltin.ToI, 0, args[0]);

            numberBase = (int)args[0].GetNumber();
            if (numberBase < 2 || numberBase > 36)
            {
                throw new InvalidOperationError(token, "Base must be between 2 and 36, inclusive.");
            }
        }

        if (value.IsString())
        {
            var stringValue = value.GetString();

            try
            {
                var parsed = ParseStringToIntBase(token, stringValue, numberBase);
                return Value.CreateInteger(parsed);
            }
            catch (Exception)
            {
                throw new ConversionError(token, $"Cannot convert non-numeric string to an integer: `{stringValue}`");
            }
        }
        else if (value.IsFloat())
        {
            // truncate decimal part
            var intValue = (long)value.GetFloat();
            return Value.CreateInteger(intValue);
        }
        else if (value.IsInteger())
        {
            // already an integer, return as-is
            return value;
        }
        else
        {
            // we can't convert
            throw new ConversionError(token, "Cannot convert non-numeric value to an integer.");
        }
    }

    /// <summary>
    /// Parses a string to a long using the specified base (2 to 36).
    /// Throws an exception if the string has invalid characters or overflows long.
    /// </summary>
    /// <param name="token">The tracing token.</param>
    /// <param name="s">The input string (e.g., "FF", "1010", "-1A").</param>
    /// <param name="numberBase">Base from 2 to 36 inclusive.</param>
    /// <returns>A long integer parsed from the string in the given base.</returns>
    private static long ParseStringToIntBase(Token token, string s, int numberBase)
    {
        s = s.Trim();

        if (string.IsNullOrEmpty(s))
        {
            throw new InvalidOperationError(token, "Empty string cannot be converted to an integer.");
        }

        var negative = false;
        if (s.StartsWith('-'))
        {
            negative = true;
            s = s[1..];
            if (string.IsNullOrEmpty(s))
            {
                // just "-" is not valid
                throw new InvalidOperationError(token, "Invalid numeric string.");
            }
        }

        var result = 0L;

        foreach (char c in s)
        {
            var digit = CharToDigit(token, c);
            if (digit < 0 || digit >= numberBase)
            {
                throw new InvalidOperationError(token, $"Invalid digit '{c}' for base {numberBase}.");
            }

            checked // to detect overflow
            {
                result = result * numberBase + digit;
            }
        }

        return negative ? -result : result;
    }

    /// <summary>
    /// Converts a single character ('0'-'9', 'a'-'z', 'A'-'Z') to its numeric value.
    /// For digits 0-9 => 0..9
    /// For letters a-z => 10..35
    /// For letters A-Z => 10..35
    /// Throws an exception if the character is not in a valid range.
    /// </summary>
    private static int CharToDigit(Token token, char c)
    {
        if (c >= '0' && c <= '9')
        {
            return c - '0';
        }

        if (c >= 'a' && c <= 'z')
        {
            return c - 'a' + 10;
        }

        if (c >= 'A' && c <= 'Z')
        {
            return c - 'A' + 10;
        }

        throw new InvalidOperationError(token, $"Invalid character '{c}' in numeric string.");
    }

    private static Value ToDate(Token token, Value value, List<Value> args)
    {
        if (args.Count != 0 && args.Count != 1)
        {
            throw new ParameterCountMismatchError(token, CoreBuiltin.ToD);
        }

        try
        {
            if (args.Count == 0)
            {
                if (value.IsString())
                {
                    if (!DateTime.TryParse(value.GetString(), out DateTime dt))
                    {
                        return Value.CreateDate(default);
                    }

                    return Value.CreateDate(dt);
                }
                else if (value.IsInteger())
                {
                    var dateTimeOffset = DateTimeOffset.FromUnixTimeMilliseconds(value.GetInteger());
                    return Value.CreateDate(dateTimeOffset.UtcDateTime);
                }
            }
            else if (args.Count == 1 && value.IsString() && args[0].IsString())
            {
                if (!DateTime.TryParseExact(value.GetString(), args[0].GetString(), null, System.Globalization.DateTimeStyles.None, out DateTime dt))
                {
                    return Value.CreateDate(default);
                }

                return Value.CreateDate(dt);
            }
        }
        catch
        {
            throw new InvalidOperationError(token, "Invalid date expression.");
        }

        throw new InvalidOperationError(token, "Invalid date expression.");
    }

    private static Value ToString(Token token, Value value, List<Value> args)
    {
        // 0 or 1 argument is allowed
        if (args.Count != 0 && args.Count != 1)
        {
            throw new ParameterCountMismatchError(token, CoreBuiltin.ToS);
        }

        // no format argument provided
        if (args.Count == 0)
        {
            var serialized = Serializer.Serialize(value);
            return Value.CreateString(serialized);
        }

        // format argument must be a string
        ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.ToS, 0, args[0]);

        var format = args[0].GetString().Trim();

        // if empty, just serialize
        if (string.IsNullOrEmpty(format))
        {
            var serialized = Serializer.Serialize(value);
            return Value.CreateString(serialized);
        }

        // numeric formatting requires integer or float
        var needsNumeric =
            format.Equals("b", StringComparison.OrdinalIgnoreCase) ||
            format.Equals("x", StringComparison.OrdinalIgnoreCase) ||
            format.Equals("o", StringComparison.OrdinalIgnoreCase) ||
            format.StartsWith("f", StringComparison.OrdinalIgnoreCase);

        if (needsNumeric && !value.IsInteger() && !value.IsFloat())
        {
            throw new InvalidOperationError(token, "Expected an integer or float.");
        }

        if (needsNumeric)
        {
            return GetFormattedNumericValue(token, value, format);
        }
        else if (value.IsDate())
        {
            return Value.CreateString(value.GetDate().ToString(format));
        }

        throw new InvalidOperationError(token, $"Unable to format type: {TypeRegistry.GetTypeName(value)}");
    }

    private static Value GetFormattedNumericValue(Token token, Value value, string format)
    {
        string formatted;
        switch (format.ToLowerInvariant())
        {
            case "b":
            case "B":
                {
                    long num = value.IsInteger() ? value.GetInteger() : (long)value.GetFloat();
                    var bin = Convert.ToString(num, 2).PadLeft(16, '0');
                    formatted = bin;
                    break;
                }

            case "x":
            case "X":
                {
                    long num = value.IsInteger() ? value.GetInteger() : (long)value.GetFloat();
                    formatted = Convert.ToString(num, 16);
                    if (format == "X")
                    {
                        // Uppercase hex
                        formatted = formatted.ToUpperInvariant();
                    }
                    break;
                }

            case "o":
            case "O":
                {
                    long num = value.IsInteger() ? value.GetInteger() : (long)value.GetFloat();
                    formatted = Convert.ToString(num, 8);
                    if (format == "O")
                    {
                        formatted = formatted.ToUpperInvariant();
                    }
                    break;
                }

            default:
                {
                    // "f" or "fN" (fixed-point)
                    if (format.StartsWith("f", StringComparison.OrdinalIgnoreCase))
                    {
                        var precisionPart = format[1..];
                        var numericVal = value.IsInteger() ? value.GetInteger() : value.GetFloat();

                        try
                        {
                            if (string.IsNullOrEmpty(precisionPart))
                            {
                                // default to 0 decimals
                                formatted = numericVal.ToString("F0", System.Globalization.CultureInfo.InvariantCulture);
                            }
                            else
                            {
                                var precision = int.Parse(precisionPart);
                                var spec = "F" + precision;
                                formatted = numericVal.ToString(spec, System.Globalization.CultureInfo.InvariantCulture);
                            }
                        }
                        catch (Exception)
                        {
                            throw new InvalidOperationError(token, $"Invalid fixed-point format `{format}`.");
                        }
                    }
                    else
                    {
                        throw new InvalidOperationError(token, $"Unknown format specifier `{format}`.");
                    }

                    break;
                }
        }

        return Value.CreateString(formatted);
    }

    private static Value Swap(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Swap, 2, args.Count);

        if (!value.IsBytes() || value.IsList())
        {
            throw new TypeError(token, $"Expected a list or bytes, but instead received `{value.Type}`.");
            
        }
        ParameterTypeMismatchError.ExpectNumber(token, CoreBuiltin.Swap, 0, args[0]);
        ParameterTypeMismatchError.ExpectNumber(token, CoreBuiltin.Swap, 1, args[1]);

        var firstIndex = (int)args[0].GetNumber();
        var secondIndex = (int)args[1].GetNumber();

        if (value.IsList())
        {
            var lst = value.GetList();

            if (firstIndex < 0 || firstIndex >= lst.Count)
            {
                throw new RangeError(token);
            }

            if (secondIndex < 0 || secondIndex >= lst.Count)
            {
                throw new RangeError(token);
            }

            var firstValue = lst[firstIndex];
            var secondValue = lst[secondIndex];
            lst[firstIndex] = secondValue;
            lst[secondIndex] = firstValue;
        }
        else if (value.IsBytes())
        {
            var lst = value.GetBytes();

            if (firstIndex < 0 || firstIndex >= lst.Length)
            {
                throw new RangeError(token);
            }

            if (secondIndex < 0 || secondIndex >= lst.Length)
            {
                throw new RangeError(token);
            }

            var firstValue = lst[firstIndex];
            var secondValue = lst[secondIndex];
            lst[firstIndex] = secondValue;
            lst[secondIndex] = firstValue;
        }

        return value;
    }

    private static Value Rotate(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Rotate, 1, args.Count);

        TypeError.ExpectList(token, value);
        ParameterTypeMismatchError.ExpectNumber(token, CoreBuiltin.Rotate, 0, args[0]);

        var elements = value.GetList();
        var rotation = (int)args[0].GetInteger();

        if (elements.Count == 0)
        {
            return value;
        }

        // normalize the rotation (mod by the list size)
        rotation %= elements.Count;
        if (rotation < 0)
        {
            // convert negative rotation to an equivalent positive rotation
            rotation += elements.Count;
        }

        var leftRotation = elements.Count - rotation;

        if (leftRotation > 0 && leftRotation < elements.Count)
        {
            var front = elements.GetRange(0, leftRotation);
            var back = elements.GetRange(leftRotation, elements.Count - leftRotation);

            elements.Clear();
            elements.AddRange(back);
            elements.AddRange(front);
        }

        return value;
    }

    private static Value Slice(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Slice, 2, args.Count);

        TypeError.ExpectList(token, value);
        ParameterTypeMismatchError.ExpectNumber(token, CoreBuiltin.Slice, 0, args[0]);
        ParameterTypeMismatchError.ExpectNumber(token, CoreBuiltin.Slice, 1, args[1]);

        var elements = value.GetList();
        var start = (int)args[0].GetNumber();
        var end = (int)args[1].GetNumber();

        if (start < 0 || end < 0 || start > end || end > elements.Count)
        {
            throw new IndexError(token);
        }

        var sliceSize = end - start;
        var slice = elements.GetRange(start, sliceSize);

        return Value.CreateList(slice);
    }

    private static Value Merge(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Merge, 1, args.Count);

        TypeError.ExpectHashmap(token, value);
        ParameterTypeMismatchError.ExpectHashmap(token, CoreBuiltin.Merge, 0, args[0]);

        var targetMap = value.GetHashmap();
        var sourceMap = args[0].GetHashmap();

        foreach (var kvp in sourceMap)
        {
            targetMap[kvp.Key] = kvp.Value;
        }

        return value;
    }

    private static Value Zip(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Zip, 1, args.Count);

        TypeError.ExpectList(token, value);
        ParameterTypeMismatchError.ExpectList(token, CoreBuiltin.Zip, 0, args[0]);

        var elements1 = value.GetList();
        var elements2 = args[0].GetList();
        var zipped = new List<Value>();
        var winMin = Math.Min(elements1.Count, elements2.Count);

        for (var i = 0; i < winMin; i++)
        {
            List<Value> pair = [elements1[i], elements2[i]];
            zipped.Add(Value.CreateList(pair));
        }

        return Value.CreateList(zipped);
    }

    private static Value Flatten(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Flatten, 0, args.Count);

        TypeError.ExpectList(token, value);

        var originalList = value.GetList();
        var flattened = FlattenListRecursive(originalList);

        return Value.CreateList(flattened);
    }

    private static List<Value> FlattenListRecursive(List<Value> input)
    {
        var result = new List<Value>();

        foreach (var item in input)
        {
            if (item.IsList())
            {
                var subList = item.GetList();
                var flattenedSubList = FlattenListRecursive(subList);
                result.AddRange(flattenedSubList);
            }
            else
            {
                result.Add(item);
            }
        }

        return result;
    }

    private static Value Insert(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Insert, 2, args.Count);

        TypeError.ExpectList(token, value);

        var lst = value.GetList();
        var index = 0;

        // note: in kiwi, the second parameter of `.insert()` is the index.
        if (!args[0].IsInteger())
        {
            throw new InvalidOperationError(token, "Expected an integer.");
        }

        index = (int)args[0].GetInteger();

        if (index >= lst.Count)
        {
            throw new IndexError(token);
        }

        lst.Insert(index, args[1]);
        return value;
    }

    private static Value Keys(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Keys, 0, args.Count);

        if (!value.IsHashmap())
        {
            throw new InvalidOperationError(token, "Expected a hashmap.");
        }

        var keys = value.GetHashmap().Keys.ToList();
        return Value.CreateList(keys);
    }

    private static Value Values(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Values, 0, args.Count);

        if (!value.IsHashmap())
        {
            throw new InvalidOperationError(token, "Expected a hashmap.");
        }

        var values = value.GetHashmap().Values.ToList();
        return Value.CreateList(values);
    }

    private static Value Enqueue(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Enqueue, 1, args.Count);

        TypeError.ExpectList(token, value);

        value.GetList().Add(args[0].Clone());
        return value;
    }

    private static Value Dequeue(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Dequeue, 0, args.Count);

        TypeError.ExpectList(token, value);

        var lst = value.GetList();

        if (lst.Count == 0)
        {
            return Value.CreateNull();
        }

        var front = lst.First().Clone();
        lst.RemoveAt(0);
        return front;
    }

    private static Value Shift(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Shift, 0, args.Count);

        TypeError.ExpectList(token, value);

        var lst = value.GetList();

        if (lst.Count == 0)
        {
            return Value.CreateNull();
        }

        var front = lst.First().Clone();
        lst.RemoveAt(0);
        return front;
    }

    private static Value Unshift(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Unshift, 1, args.Count);

        TypeError.ExpectList(token, value);

        value.GetList().Insert(0, args[0].Clone());
        return value;
    }

    private static Value Concat(Token token, Value value, List<Value> args)
    {
        if (args.Count == 0)
        {
            throw new ParameterCountMismatchError(token, CoreBuiltin.Concat);
        }

        if (value.IsString())
        {
            System.Text.StringBuilder sv = new();
            sv.Append(value.GetString());

            foreach (var arg in args)
            {
                sv.Append(Serializer.Serialize(arg));
            }

            return Value.CreateString(sv.ToString());
        }
        else if (value.IsList())
        {
            var lst = value.GetList();

            foreach (var arg in args)
            {
                if (arg.IsList())
                {
                    lst.AddRange(arg.Clone().GetList());
                }
                else
                {
                    lst.Add(arg.Clone());
                }
            }

            return value;
        }

        throw new InvalidOperationError(token, "Expected a string or list.");
    }

    private static Value Unique(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Unique, 0, args.Count);

        TypeError.ExpectList(token, value);

        var originalList = value.GetList();
        var uniqueList = new List<Value>();
        var seen = new HashSet<Value>();

        foreach (var item in originalList)
        {
            if (seen.Add(item))
            {
                uniqueList.Add(item);
            }
        }

        return Value.CreateList(uniqueList);
    }

    private static Value Count(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Count, 1, args.Count);

        if (value.IsString())
        {
            ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.Count, 0, args[0]);

            var s = value.GetString();
            var target = args[0].GetString();
            var count = 0;

            if (string.IsNullOrEmpty(s) || string.IsNullOrEmpty(target) || target.Length > s.Length)
            {
                throw new InvalidOperationError(token, "Expected source string to be non-empty and longer than target string.");
            }

            for (var i = 0; i <= s.Length - target.Length; i++)
            {
                if (s.Substring(i, target.Length) == target)
                {
                    count++;
                }
            }

            return Value.CreateInteger(count);
        }
        else if (value.IsList())
        {
            var list = value.GetList();
            var count = 0;

            foreach (var item in list)
            {
                if (item.Equals(args[0]))
                {
                    count++;
                }
            }

            return Value.CreateInteger(count);
        }
        else if (value.IsBytes())
        {
            TypeError.ExpectInteger(token, args[0]);
            ByteCheck(token, args[0].GetInteger());

            var count = 0;
            var byteValue = (byte)args[0].GetInteger();

            foreach (var byt in value.GetBytes())
            {
                if (byt == byteValue)
                {
                    count++;
                }
            }

            return Value.CreateInteger(count);
        }

        throw new InvalidOperationError(token, "Expected a string or list.");
    }

    private static Value Substring(Token token, Value value, List<Value> args)
    {
        if (args.Count != 1 && args.Count != 2)
        {
            throw new ParameterCountMismatchError(token, CoreBuiltin.Substring);
        }

        TypeError.ExpectString(token, value);

        var s = value.GetString();
        var index = (int)args[0].GetInteger();
        var size = s.Length;

        if (args.Count == 2)
        {
            ParameterTypeMismatchError.ExpectInteger(token, CoreBuiltin.Substring, 1, args[1]);

            size = (int)args[1].GetInteger();
        }

        return Value.CreateString(s.Substring(index, Math.Min(size, s.Length - index)));
    }

    private static Value Reverse(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Reverse, 0, args.Count);

        if (value.IsString())
        {
            return Value.CreateString(string.Join(string.Empty, value.GetString().Reverse()));
        }
        else if (value.IsList())
        {
            value.GetList().Reverse();
            return value;
        }
        else if (value.IsBytes())
        {
            var lst = value.GetBytes().ToList();
            lst.Reverse();
            return Value.CreateBytes([.. lst]);
        }

        throw new InvalidOperationError(token, "Expected a string, list, or bytes.");
    }

    private static Value IndexOf(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.IndexOf, 1, args.Count);

        if (value.IsString())
        {
            ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.IndexOf, 0, args[0]);

            return Value.CreateInteger(value.GetString().IndexOf(args[0].GetString()));
        }
        else if (value.IsList())
        {
            return Value.CreateInteger(value.GetList().IndexOf(args[0]));
        }

        throw new InvalidOperationError(token, "Expected a string or list.");
    }

    private static Value LastIndexOf(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.LastIndexOf, 1, args.Count);

        if (value.IsString())
        {
            ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.LastIndexOf, 0, args[0]);

            return Value.CreateInteger(value.GetString().LastIndexOf(args[0].GetString()));
        }
        else if (value.IsList())
        {
            return Value.CreateInteger(value.GetList().LastIndexOf(args[0]));
        }

        throw new InvalidOperationError(token, "Expected a string or list.");
    }

    private static Value Clone(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Clone, 0, args.Count);

        return value.Clone();
    }

    private static Value Lines(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Lines, 0, args.Count);

        TypeError.ExpectString(token, value);

        return Value.CreateList([.. value.GetString().Split(Environment.NewLine, StringSplitOptions.None).Select(x => Value.CreateString(x))]);
    }

    private static Value Split(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Split, 1, args.Count);

        TypeError.ExpectString(token, value);

        var s = value.GetString();

        if (string.IsNullOrEmpty(s))
        {
            return Value.CreateList([]);
        }

        ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.Split, 0, args[0]);

        var delim = args[0].GetString();

        if (string.IsNullOrEmpty(delim))
        {
            var chars = s.ToCharArray().Select(x => Value.CreateString(x.ToString())).ToList();
            return Value.CreateList(chars);
        }
        else
        {
            var tokens = s.Split(delim, StringSplitOptions.None).Select(x => Value.CreateString(x)).ToList();
            return Value.CreateList(tokens);
        }
    }

    private static Value Truthy(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Truthy, 0, args.Count);

        return Value.CreateBoolean(BooleanOp.IsTruthy(value));
    }

    private static Value Get(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Get, 1, args.Count);

        if (value.IsList())
        {
            ParameterTypeMismatchError.ExpectInteger(token, CoreBuiltin.Get, 0, args[0]);

            var index = (int)args[0].GetInteger();

            if (index < 0 || index >= value.GetList().Count)
            {
                throw new IndexError(token);
            }

            return value.GetList()[index];
        }
        else if (value.IsBytes())
        {
            ParameterTypeMismatchError.ExpectInteger(token, CoreBuiltin.Get, 0, args[0]);

            var index = (int)args[0].GetInteger();

            if (index < 0 || index >= value.GetBytes().Length)
            {
                throw new IndexError(token);
            }

            return Value.CreateInteger(value.GetBytes()[index]);
        }
        else if (value.IsHashmap())
        {
            if (value.GetHashmap().TryGetValue(args[0], out Value? v))
            {
                return v;
            }
            else
            {
                return Value.CreateNull();
            }
        }

        throw new InvalidOperationError(token, "Expected a list or hashmap.");
    }

    private static Value Set(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Set, 2, args.Count);

        if (value.IsList())
        {
            ParameterTypeMismatchError.ExpectInteger(token, CoreBuiltin.Set, 0, args[0]);

            var index = (int)args[0].GetInteger();

            if (index < 0 || index >= value.GetList().Count)
            {
                throw new IndexError(token);
            }

            value.GetList()[index] = args[1].Clone();
            return value;
        }
        else if (value.IsBytes())
        {
            ParameterTypeMismatchError.ExpectInteger(token, CoreBuiltin.Set, 0, args[0]);

            var index = (int)args[0].GetInteger();

            if (index < 0 || index >= value.GetBytes().Length)
            {
                throw new IndexError(token);
            }

            ParameterTypeMismatchError.ExpectInteger(token, CoreBuiltin.Set, 1, args[1]);
            var byteValue = args[1].GetInteger();
            ByteCheck(token, byteValue);

            value.GetBytes()[index] = (byte)args[1].GetInteger();
            return value;
        }
        else if (value.IsHashmap())
        {
            value.GetHashmap()[args[0]] = args[1].Clone();
            return value;
        }

        throw new InvalidOperationError(token, "Expected a list or hashmap.");
    }

    private static void ByteCheck(Token token, long byteValue)
    {
        if (byteValue < 0 || byteValue > 255)
        {
            throw new InvalidOperationError(token, $"Byte range is from 0 to 255 inclusive, but received {byteValue}");
        }
    }

    private static Value Remove(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Remove, 1, args.Count);

        if (value.IsList())
        {
            value.GetList().Remove(args[0]);
            return value;
        }
        else if (value.IsHashmap())
        {
            value.GetHashmap().Remove(args[0]);
            return value;
        }

        throw new InvalidOperationError(token, "Expected a list or hashmap.");
    }

    private static Value RemoveAt(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.RemoveAt, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, CoreBuiltin.RemoveAt, 0, args[0]);
        var index = (int)args[0].GetInteger();

        if (value.IsString())
        {
            var str = value.GetString();

            if (index < 0)
            {
                index = str.Length + index;
            }

            if (index < 0 || index >= str.Length)
            {
                throw new IndexError(token, "Index out of bounds.");
            }

            return Value.CreateString(str.Remove(index, 1));
        }
        else if (value.IsList())
        {
            var lst = value.GetList();

            if (index < 0)
            {
                index = lst.Count + index;
            }

            if (index < 0 || index >= lst.Count)
            {
                throw new IndexError(token, "Index out of bounds.");
            }

            value.GetList().RemoveAt(index);
            return value;
        }
        else if (value.IsBytes())
        {
            var lst = value.GetBytes().ToList();

            if (index < 0)
            {
                index = lst.Count + index;
            }

            if (index < 0 || index >= lst.Count)
            {
                throw new IndexError(token, "Index out of bounds.");
            }

            lst.RemoveAt(index);
            return Value.CreateBytes([.. lst]);
        }

        throw new InvalidOperationError(token, "Expected a string, list, or bytes.");
    }

    private static Value Replace(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Replace, 2, args.Count);

        var find = args[0];
        var replace = args[1];

        if (value.IsString())
        {
            if (!find.IsString() && !replace.IsString())
            {
                throw new InvalidOperationError(token, "Expected strings.");
            }

            return Value.CreateString(value.GetString().Replace(find.GetString(), replace.GetString()));
        }
        else if (value.IsList())
        {
            List<Value> lst = [];

            foreach (var item in value.GetList())
            {
                if (item.Equals(find))
                {
                    lst.Add(replace);
                }
                else
                {
                    lst.Add(item);
                }
            }

            return Value.CreateList(lst);
        }

        throw new InvalidOperationError(token, "Expected a string or a list.");
    }

    private static Value Clear(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Clear, 0, args.Count);

        if (value.IsString())
        {
            value.SetValue(string.Empty);
            return value;
        }
        else if (value.IsList())
        {
            value.GetList().Clear();
            return value;
        }
        else if (value.IsHashmap())
        {
            value.GetHashmap().Clear();
            return value;
        }
        else if (value.IsBytes())
        {
            value.SetValue(""u8.ToArray());
            return value;
        }

        throw new InvalidOperationError(token, "Expected a string, list, hashmap, or bytes.");
    }

    private static Value First(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.First, 0, args.Count);

        if (value.IsList())
        {
            var lst = value.GetList();

            if (lst.Count != 0)
            {
                return lst[0];
            }
        }
        else if (value.IsBytes())
        {
            var lst = value.GetBytes();

            if (lst.Length != 0)
            {
                return Value.CreateInteger(lst[0]);
            }            
        }

        return Value.CreateNull();
    }

    private static Value Last(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Last, 0, args.Count);

        if (value.IsList())
        {
            var lst = value.GetList();

            if (lst.Count != 0)
            {
                return lst[^1];
            }            
        }
        else if (value.IsBytes())
        {
            var lst = value.GetBytes();

            if (lst.Length != 0)
            {
                return Value.CreateInteger(lst[^1]);
            }
        }

        return Value.CreateNull();
    }

    private static Value Push(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Push, 1, args.Count);

        if (value.IsList())
        {
            value.GetList().Add(args[0].Clone());            
        }
        else if (value.IsBytes())
        {
            TypeError.ExpectInteger(token, args[0]);
            var byteValue = args[0].GetInteger();
            ByteCheck(token, byteValue);

            var byteList = value.GetBytes().ToList();
            byteList.Add((byte)byteValue);
            value.SetValue([.. byteList]);
        }

        return value;
    }

    private static Value Pop(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Pop, 0, args.Count);
        
        if (value.IsList())
        {
            var lst = value.GetList();
            if (lst.Count == 0)
            {
                return Value.CreateNull();
            }

            var back = lst[^1].Clone();
            lst.RemoveAt(lst.Count - 1);
            return back;
        }
        else if (value.IsBytes())
        {
            var lst = value.GetBytes().ToList();
            if (lst.Count == 0)
            {
                return Value.CreateNull();
            }

            var back = lst[^1];
            lst.RemoveAt(lst.Count - 1);
            value.SetValue([.. lst]);
            return Value.CreateInteger(back);
        }

        throw new InvalidOperationError(token, "Expected a list or bytes.");
    }

    private static Value ToList(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.ToList, 0, args.Count);

        if (value.IsList())
        {
            return value;
        }
        else if (value.IsString())
        {
            var s = value.GetString();

            if (string.IsNullOrEmpty(s))
            {
                return Value.CreateList([]);
            }

            var chars = s.ToCharArray().Select(x => Value.CreateString(x.ToString())).ToList();
            return Value.CreateList(chars);
        }
        else if (value.IsBytes())
        {
            if (value.GetBytes().Length == 0)
            {
                return Value.CreateList([]);
            }

            var bytes = value.GetBytes().ToList().Select(x => Value.CreateInteger((int)x)).ToList();
            return Value.CreateList(bytes);
        }

        throw new InvalidOperationError(token, "Expected a string, list, or bytes.");
    }

    private static Value Chars(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Chars, 0, args.Count);
        TypeError.ExpectString(token, value);

        var s = value.GetString();

        if (string.IsNullOrEmpty(s))
        {
            return Value.CreateList([]);
        }

        var chars = s.ToCharArray().Select(x => Value.CreateString(x.ToString())).ToList();
        return Value.CreateList(chars);
    }

    private static Value Chomp(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Chomp, 0, args.Count);

        TypeError.ExpectString(token, value);

        var input = value.GetString();

        if (string.IsNullOrEmpty(input))
        {
            value = Value.EmptyString;
        }
        else if (input.EndsWith("\r\n"))
        {
            value = Value.CreateString(input[..^2]);
        }
        else if (input.EndsWith('\n') || input.EndsWith('\r'))
        {
            value = Value.CreateString(input[..^1]);
        }

        return value;
    }

    private static Value HasKey(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.HasKey, 1, args.Count);

        TypeError.ExpectHashmap(token, value);

        return Value.CreateBoolean(value.GetHashmap().ContainsKey(args[0]));
    }

    private static Value Join(Token token, Value value, List<Value> args)
    {
        if (args.Count != 0 && args.Count != 1)
        {
            throw new ParameterCountMismatchError(token, CoreBuiltin.Join);
        }

        TypeError.ExpectList(token, value);

        var joiner = string.Empty;

        if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.Join, 0, args[0]);

            joiner = args[0].GetString();
        }

        System.Text.StringBuilder sv = new();

        var first = true;
        foreach (var item in value.GetList())
        {
            if (!first)
            {
                sv.Append(joiner);
            }
            else
            {
                first = false;
            }
            sv.Append(Serializer.Serialize(item));
        }

        return Value.CreateString(sv.ToString());
    }

    private static Value Size(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Size, 0, args.Count);

        if (value.IsString())
        {
            return Value.CreateInteger(value.GetString().Length);
        }
        else if (value.IsList())
        {
            return Value.CreateInteger(value.GetList().Count);
        }
        else if (value.IsHashmap())
        {
            return Value.CreateInteger(value.GetHashmap().Count);
        }
        else if (value.IsBytes())
        {
            return Value.CreateInteger(value.GetBytes().Length);
        }

        throw new InvalidOperationError(token, "Expected a string, list, hashmap, or bytes.");
    }

    private static Value Empty(Token token, Value value, List<Value> args)
    {
        if (args.Count != 0 && args.Count != 1)
        {
            throw new ParameterCountMismatchError(token, CoreBuiltin.Empty);
        }

        var isEmpty = false;
        if (value.IsString())
        {
            isEmpty = string.IsNullOrEmpty(value.GetString());
        }
        else if (value.IsList())
        {
            isEmpty = value.GetList().Count == 0;
        }
        else if (value.IsBytes())
        {
            isEmpty = value.GetBytes().Length == 0;
        }
        else if (value.IsHashmap())
        {
            isEmpty = value.GetHashmap().Count == 0;
        }
        else if (value.IsInteger())
        {
            isEmpty = value.GetInteger() == 0L;
        }
        else if (value.IsFloat())
        {
            isEmpty = value.GetFloat() == 0D;
        }
        else if (value.IsBoolean())
        {
            isEmpty = !value.GetBoolean();
        }
        else if (value.IsNull())
        {
            isEmpty = true;
        }
        else
        {
            throw new InvalidOperationError(token, "Type cannot be empty.");
        }

        if (args.Count == 0)
        {
            return Value.CreateBoolean(isEmpty);
        }

        if (isEmpty)
        {
            return args[0];
        }

        return value;
    }

    private static Value Lowercase(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Lowercase, 0, args.Count);

        TypeError.ExpectString(token, value);

        return Value.CreateString(value.GetString().ToLower());
    }

    private static Value Uppercase(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Uppercase, 0, args.Count);

        TypeError.ExpectString(token, value);

        return Value.CreateString(value.GetString().ToUpper());
    }

    private static Value LeftTrim(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.LeftTrim, 0, args.Count);

        TypeError.ExpectString(token, value);

        return Value.CreateString(value.GetString().TrimStart());
    }

    private static Value RightTrim(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.RightTrim, 0, args.Count);

        TypeError.ExpectString(token, value);

        return Value.CreateString(value.GetString().TrimEnd());
    }

    private static Value Trim(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Trim, 0, args.Count);

        TypeError.ExpectString(token, value);

        return Value.CreateString(value.GetString().Trim());
    }

    private static Value Type(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Type, 0, args.Count);

        return Value.CreateString(TypeRegistry.GetTypeName(value));
    }

    private static Value BeginsWith(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.BeginsWith, 1, args.Count);

        TypeError.ExpectString(token, value);
        ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.BeginsWith, 0, args[0]);

        return Value.CreateBoolean(value.GetString().StartsWith(args[0].GetString()));
    }

    private static Value Contains(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.Contains, 1, args.Count);

        if (value.IsString())
        {
            ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.Contains, 0, args[0]);

            return Value.CreateBoolean(value.GetString().Contains(args[0].GetString()));
        }
        else if (value.IsList())
        {
            return Value.CreateBoolean(value.GetList().Contains(args[0]));
        }
        else if (value.IsBytes())
        {
            ParameterTypeMismatchError.ExpectInteger(token, CoreBuiltin.Contains, 0, args[0]);
            ByteCheck(token, args[0].GetInteger());
            var byteValue = (byte)args[0].GetInteger();

            foreach (var byt in value.GetBytes())
            {
                if (byt == byteValue)
                {
                    return Value.CreateBoolean(true);
                }
            }

            return Value.CreateBoolean(false);
        }

        throw new InvalidOperationError(token, "Expected a string or list.");
    }

    private static Value EndsWith(Token token, Value value, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, CoreBuiltin.EndsWith, 1, args.Count);

        TypeError.ExpectString(token, value);
        ParameterTypeMismatchError.ExpectString(token, CoreBuiltin.EndsWith, 0, args[0]);

        return Value.CreateBoolean(value.GetString().EndsWith(args[0].GetString()));
    }
}