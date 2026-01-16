using System.Runtime.InteropServices;
using System.Text;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;
public static class ConsoleBuiltinHandler
{
    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Console_Input => Input(token, args),
            TokenName.Builtin_Console_Foreground => Foreground(token, args),
            TokenName.Builtin_Console_Background => Background(token, args),
            TokenName.Builtin_Console_Clear => Clear(token, args),
            TokenName.Builtin_Console_CursorVisible => CursorVisible(token, args),
            TokenName.Builtin_Console_ReadKey => ReadKey(token, args),
            TokenName.Builtin_Console_Reset => Reset(token, args),
            TokenName.Builtin_Console_Title => Title(token, args),
            TokenName.Builtin_Console_WindowSize => WindowSize(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Clear(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.Clear, 0, args.Count);

        Console.Clear();

        return Value.Default;
    }

    private static Value Reset(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.Clear, 0, args.Count);

        Console.ResetColor();

        return Value.Default;
    }

    private static Value ReadKey(Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, ConsoleBuiltin.ReadKey, "0–1", args.Count);
        }

        bool intercept = true;
        if (args.Count == 1)
        {
            ParameterTypeMismatchError.ExpectBoolean(token, ConsoleBuiltin.ReadKey, 0, args[0]);
            intercept = args[0].GetBoolean();
        }

        ConsoleKeyInfo keyInfo;
        try
        {
            keyInfo = Console.ReadKey(intercept);
        }
        catch (Exception ex)
        {
            throw new KiwiError(token, $"Failed to read key: {ex.Message}");
        }

        Dictionary<Value, Value> resultMap = [];
        resultMap[Value.CreateString("key")] = Value.CreateString(keyInfo.Key.ToString());

        char ch = keyInfo.KeyChar;
        string charStr = char.IsControl(ch) || ch == '\0' ? "" : ch.ToString();
        resultMap[Value.CreateString("char")] = Value.CreateString(charStr);

        List<Value> mods = [];
        if ((keyInfo.Modifiers & ConsoleModifiers.Shift) != 0)
        {
            mods.Add(Value.CreateString("Shift"));
        }
        
        if ((keyInfo.Modifiers & ConsoleModifiers.Control) != 0)
        {
            mods.Add(Value.CreateString("Control"));
        }

        if ((keyInfo.Modifiers & ConsoleModifiers.Alt) != 0)
        {
            mods.Add(Value.CreateString("Alt"));
        }

        resultMap[Value.CreateString("modifiers")] = Value.CreateList(mods);
        resultMap[Value.CreateString("is_shift")] = Value.CreateBoolean((keyInfo.Modifiers & ConsoleModifiers.Shift) != 0);
        resultMap[Value.CreateString("is_control")] = Value.CreateBoolean((keyInfo.Modifiers & ConsoleModifiers.Control) != 0);
        resultMap[Value.CreateString("is_alt")] = Value.CreateBoolean((keyInfo.Modifiers & ConsoleModifiers.Alt) != 0);
        resultMap[Value.CreateString("is_printable")] = Value.CreateBoolean(!char.IsControl(keyInfo.KeyChar) && keyInfo.KeyChar != '\0');

        return Value.CreateHashmap(resultMap);
    }

    private static Value Input(Token token, List<Value> args)
    {
        if (args.Count > 3)
        {
            throw new ParameterCountMismatchError(token, ConsoleBuiltin.Input, "0–3", args.Count);
        }

        string? prompt = null;
        bool noEcho = false;
        string? mask = null;

        // Parse arguments
        if (args.Count >= 1)
        {
            ParameterTypeMismatchError.ExpectString(token, ConsoleBuiltin.Input, 0, args[0]);
            prompt = args[0].GetString();
        }

        if (args.Count >= 2)
        {
            ParameterTypeMismatchError.ExpectBoolean(token, ConsoleBuiltin.Input, 1, args[1]);
            noEcho = args[1].GetBoolean();
        }

        if (args.Count == 3)
        {
            ParameterTypeMismatchError.ExpectString(token, ConsoleBuiltin.Input, 2, args[2]);
            var maskStr = args[2].GetString();
            // take first char only
            mask = string.IsNullOrEmpty(maskStr) ? null : maskStr[0].ToString();
        }

        if (prompt != null)
        {
            Console.Write(prompt);
        }

        string result;

        if (noEcho)
        {
            result = ReadLineSilent(token, mask);
            Console.WriteLine(); // add newline after silent read
        }
        else
        {
            result = Console.ReadLine() ?? string.Empty;
        }

        return Value.CreateString(result);
    }

    private static Value Title(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.Title, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, ConsoleBuiltin.Title, 0, args[0]);
        Console.Title = args[0].GetString();
        return Value.Default;
    }

    private static Value CursorVisible(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.CursorVisible, 1, args.Count);
        ParameterTypeMismatchError.ExpectBoolean(token, ConsoleBuiltin.CursorVisible, 0, args[0]);
        Console.CursorVisible = args[0].GetBoolean();
        return Value.Default;
    }

    private static Value WindowSize(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.WindowSize, 0, args.Count);

        var map = new Dictionary<Value, Value>
        {
            [Value.CreateString("width")]  = Value.CreateInteger(Console.WindowWidth),
            [Value.CreateString("height")] = Value.CreateInteger(Console.WindowHeight)
        };

        return Value.CreateHashmap(map);
    }

    private static Value Foreground(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.Foreground, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, ConsoleBuiltin.Foreground, 0, args[0]);

        Console.ForegroundColor = (ConsoleColor)args[0].GetInteger();

        return Value.Default;
    }

    private static Value Background(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ConsoleBuiltin.Background, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, ConsoleBuiltin.Background, 0, args[0]);

        Console.BackgroundColor = (ConsoleColor)args[0].GetInteger();

        return Value.Default;
    }

    private static string ReadLineSilent(Token token, string? mask = null)
    {
        var sb = new StringBuilder();

        bool isWindows = RuntimeInformation.IsOSPlatform(OSPlatform.Windows);

        if (isWindows)
        {
            while (true)
            {
                var key = Console.ReadKey(intercept: true);
                if (key.Key == ConsoleKey.Enter) break;
                if (key.Key == ConsoleKey.Backspace && sb.Length > 0)
                {
                    sb.Length--;
                    if (mask != null)
                    {
                        Console.Write("\b \b");
                    }
                    continue;
                }
                if (!char.IsControl(key.KeyChar))
                {
                    sb.Append(key.KeyChar);
                    if (mask != null)
                    {
                        Console.Write(mask);
                    }
                }
            }
        }
        else
        {
            Termios original;
            GetTermios(token, out original);

            try
            {
                var modified = original;
                modified.c_lflag &= ~ECHO; // disable echo
                
                SetTermios(token, ref modified);

                while (true)
                {
                    var key = Console.ReadKey(intercept: true);
                    if (key.Key == ConsoleKey.Enter)
                    {
                        break;
                    }

                    if (key.Key == ConsoleKey.Backspace && sb.Length > 0)
                    {
                        sb.Length--;
                        
                        if (mask != null)
                        {
                            Console.Write("\b \b");
                        }
                        
                        continue;
                    }

                    if (!char.IsControl(key.KeyChar))
                    {
                        sb.Append(key.KeyChar);
                        if (mask != null)
                        {
                            Console.Write(mask);
                        }
                    }
                }
            }
            finally
            {
                SetTermios(token, ref original); // restore
            }
        }

        return sb.ToString();
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct Termios
    {
        public uint c_iflag;
        public uint c_oflag;
        public uint c_cflag;
        public uint c_lflag;

        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 20)]
        public byte[] c_cc; // control chars
    }

    private const uint ECHO = 0x00000008u; // from termios.h
    private const int TCSANOW = 0;

    [DllImport("libc", SetLastError = true)]
    private static extern int tcgetattr(int fd, out Termios termios);

    [DllImport("libc", SetLastError = true)]
    private static extern int tcsetattr(int fd, int optional_actions, ref Termios termios);

    private static void GetTermios(Token token, out Termios term)
    {
        if (tcgetattr(0, out term) != 0)
        {
            throw new InvalidOperationError(token, "Failed to get terminal attributes");
        }
    }

    private static void SetTermios(Token token, ref Termios term)
    {
        if (tcsetattr(0, TCSANOW, ref term) != 0)
        {
            throw new InvalidOperationError(token, "Failed to set terminal attributes");
        }
    }
}