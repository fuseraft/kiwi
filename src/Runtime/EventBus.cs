using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime;

public sealed class EventBus
{
    // Handlers keyed by registration pattern (may contain '*' wildcards).
    // Each entry is kept sorted descending by priority so high-priority handlers fire first.
    private readonly Dictionary<string, List<Handler>> _handlers = [];

    private readonly record struct Handler(Value Callback, bool Once, int Priority);

    public void On(string pattern, Value callback, bool once = false, int priority = 0)
    {
        if (!_handlers.TryGetValue(pattern, out var list))
        {
            list = [];
            _handlers[pattern] = list;
        }

        // Insert in descending priority order (stable — new handlers with equal priority go last).
        var idx = list.FindLastIndex(h => h.Priority >= priority);
        list.Insert(idx + 1, new Handler(callback, once, priority));
    }

    public void Once(string pattern, Value callback, int priority = 0) =>
        On(pattern, callback, once: true, priority);

    public void Off(string pattern, Value? callback = null)
    {
        if (!_handlers.TryGetValue(pattern, out var list))
            return;

        if (callback == null || callback.IsNull())
        {
            list.Clear();
        }
        else if (callback.IsLambda())
        {
            var id = callback.GetLambda().Identifier;
            list.RemoveAll(h => h.Callback.IsLambda() && h.Callback.GetLambda().Identifier == id);
        }
    }

    /// <summary>
    /// Fires all handlers whose registration pattern matches <paramref name="eventName"/>.
    /// <para>
    /// - <c>once</c> handlers are removed <em>before</em> invocation so they are never
    ///   called twice, even if the handler itself throws.
    /// - All matching handlers are invoked regardless of individual failures; errors are
    ///   collected and re-thrown as a single <see cref="EventError"/> after the full
    ///   dispatch round completes.
    /// - Return values from every handler are collected into a list and returned to the
    ///   caller, enabling request/response patterns.
    /// </para>
    /// </summary>
    public List<Value> Emit(Interpreter interp, Token token, string eventName, List<Value> data)
    {
        var results = new List<Value>();
        List<string>? errors = null;

        foreach (var (pattern, list) in _handlers)
        {
            if (!Matches(pattern, eventName))
                continue;

            // Snapshot handlers that should fire, then purge 'once' entries up-front
            // so they are guaranteed gone even when a handler throws.
            var snapshot = list.ToList();
            list.RemoveAll(h => h.Once);

            foreach (var handler in snapshot)
            {
                if (!handler.Callback.IsLambda())
                    continue;

                try
                {
                    results.Add(interp.InvokeEvent(token, handler.Callback.GetLambda(), data));
                }
                catch (Exception ex)
                {
                    (errors ??= []).Add(ex.Message);
                }
            }
        }

        if (errors != null)
        {
            var msg = errors.Count == 1
                ? errors[0]
                : string.Join("; ", errors.Select((e, i) => $"[{i + 1}] {e}"));
            throw new EventError(token, $"Event handler error ({eventName}): {msg}");
        }

        return results;
    }

    // -------------------------------------------------------------------------
    // Pattern matching
    // -------------------------------------------------------------------------

    /// <summary>
    /// Matches a registration pattern against a concrete event name.
    /// <c>*</c> matches any sequence of non-<c>.</c> characters within a single
    /// dot-separated segment. Examples:
    /// <list type="bullet">
    ///   <item><c>user.*</c>  matches <c>user.login</c>, <c>user.logout</c></item>
    ///   <item><c>*.error</c> matches <c>db.error</c>, <c>app.error</c></item>
    ///   <item><c>*</c>       matches any single-segment name</item>
    /// </list>
    /// Wildcards do NOT cross segment boundaries; use separate registrations for
    /// deeper nesting.
    /// </summary>
    public static bool Matches(string pattern, string eventName)
    {
        if (pattern == eventName) return true;
        if (!pattern.Contains('*')) return false;
        return MatchGlob(pattern.AsSpan(), eventName.AsSpan());
    }

    private static bool MatchGlob(ReadOnlySpan<char> pattern, ReadOnlySpan<char> name)
    {
        while (true)
        {
            if (pattern.IsEmpty) return name.IsEmpty;

            if (pattern[0] == '*')
            {
                pattern = pattern[1..];
                // '*' matches zero or more non-'.' characters — never crosses a segment boundary.
                for (int i = 0; i <= name.Length; i++)
                {
                    if (i > 0 && name[i - 1] == '.') break;
                    if (MatchGlob(pattern, name[i..])) return true;
                }
                return false;
            }

            if (name.IsEmpty || pattern[0] != name[0]) return false;
            pattern = pattern[1..];
            name = name[1..];
        }
    }
}
