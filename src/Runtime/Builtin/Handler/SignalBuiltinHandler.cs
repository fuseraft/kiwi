using System.Collections.Concurrent;
using System.Runtime.InteropServices;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Settings;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class SignalBuiltinHandler
{
    // Maps signal name → pending count (>0 means fired since last check)
    private static readonly ConcurrentDictionary<string, long> _fired    = new();
    private static readonly ConcurrentDictionary<string, bool> _trapped  = new();

    // Hold references to PosixSignalRegistration so they're not GC'd
    private static readonly List<PosixSignalRegistration> _registrations = [];
    private static readonly Lock _regLock = new();

    private static bool _cancelKeyRegistered;

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        if (Kiwi.Settings.SafeMode) throw new SafeModeError(token, "signal");
        return builtin switch
        {
            TokenName.Builtin_Signal_Trap  => Trap(token, args),
            TokenName.Builtin_Signal_Check => Check(token, args),
            TokenName.Builtin_Signal_Reset => Reset(token, args),
            TokenName.Builtin_Signal_Raise => Raise(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Trap(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SignalBuiltin.Trap, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SignalBuiltin.Trap, 0, args[0]);

        var sigName = args[0].GetString().ToUpperInvariant();
        _fired[sigName]   = 0;
        _trapped[sigName] = true;

        RegisterOsHandler(sigName);
        return Value.Default;
    }

    private static Value Check(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SignalBuiltin.Check, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SignalBuiltin.Check, 0, args[0]);

        var sigName = args[0].GetString().ToUpperInvariant();

        if (_fired.TryGetValue(sigName, out var count) && count > 0)
        {
            _fired.AddOrUpdate(sigName, 0, (_, _) => 0);
            return Value.CreateBoolean(true);
        }

        return Value.CreateBoolean(false);
    }

    private static Value Reset(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SignalBuiltin.Reset, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SignalBuiltin.Reset, 0, args[0]);

        var sigName = args[0].GetString().ToUpperInvariant();
        _trapped.TryRemove(sigName, out _);
        _fired.TryRemove(sigName, out _);
        return Value.Default;
    }

    private static Value Raise(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SignalBuiltin.Raise, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SignalBuiltin.Raise, 0, args[0]);

        var sigName = args[0].GetString().ToUpperInvariant();
        _fired.AddOrUpdate(sigName, 1, (_, v) => v + 1);
        return Value.Default;
    }

    private static void RegisterOsHandler(string sigName)
    {
        // Register SIGINT via CancelKeyPress (cross-platform)
        if (sigName == "SIGINT" && !_cancelKeyRegistered)
        {
            _cancelKeyRegistered = true;
            Console.CancelKeyPress += (_, e) =>
            {
                e.Cancel = true;
                _fired.AddOrUpdate("SIGINT", 1, (_, v) => v + 1);
            };
        }

        // Register POSIX signals on non-Windows platforms
        if (!RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
        {
            PosixSignal? posixSig = sigName switch
            {
                "SIGTERM" => PosixSignal.SIGTERM,
                "SIGHUP"  => PosixSignal.SIGHUP,
                // SIGUSR1 (10) and SIGUSR2 (12) are not in PosixSignal enum on all runtimes;
                // they are handled via __signal_raise__ programmatically.
                _ => null
            };

            if (posixSig.HasValue)
            {
                var captured = sigName;
                var reg = PosixSignalRegistration.Create(posixSig.Value, _ =>
                {
                    _fired.AddOrUpdate(captured, 1, (_, v) => v + 1);
                });

                lock (_regLock)
                    _registrations.Add(reg);
            }
        }
    }
}
