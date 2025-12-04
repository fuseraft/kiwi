using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime;

public sealed class EventBus
{
    private readonly Dictionary<string, List<(Value Callback, bool Once)>> _handlers = [];

    public Dictionary<string, List<(Value Callback, bool Once)>> Handlers => _handlers;

    public void On(string name, Value callback, bool once = false)
    {
        if (!_handlers.TryGetValue(name, out var list))
        {
            list = [];
            _handlers[name] = list;
        }

        list.Add((callback, once));
    }

    public void Once(string name, Value callback) => On(name, callback, once: true);

    public void Off(string name, Value? callback = null)
    {
        if (!_handlers.TryGetValue(name, out var list))
        {
            return;
        }

        if (callback == null || callback.IsNull())
        {
            // Remove ALL handlers for this event
            list.Clear();
        }
        else
        {
            list.RemoveAll(h => h.Callback.IsLambda() == callback.IsLambda() && h.Callback.GetLambda().Identifier == callback.GetLambda().Identifier);
        }
    }

    public void Emit(Token token, string name, List<Value> data)
    {
        if (!_handlers.TryGetValue(name, out var list))
        {
            return;
        }

        List<(Value, bool)> toRemove = [];

        foreach (var (cb, once) in list)
        {
            if (once) 
            {
                toRemove.Add((cb, once));
            }

            try
            {
                if (cb.IsLambda())
                {
                    Interpreter.Current?.InvokeEvent(token, cb.GetLambda(), data);
                }
            }
            catch (Exception ex)
            {
                throw new EventError(token, $"Event handler error ({name}): {ex.Message}");
            }
        }

        foreach (var item in toRemove)
        {
            list.Remove(item);
        }
    }
}