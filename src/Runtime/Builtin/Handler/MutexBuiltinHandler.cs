using System.Collections.Concurrent;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class MutexBuiltinHandler
{
    private static long _nextId;
    private static readonly ConcurrentDictionary<long, SemaphoreSlim> _mutexes = new();

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Mutex_Create     => Create(token, args),
            TokenName.Builtin_Mutex_Acquire    => Acquire(token, args),
            TokenName.Builtin_Mutex_TryAcquire => TryAcquire(token, args),
            TokenName.Builtin_Mutex_Release    => Release(token, args),
            TokenName.Builtin_Mutex_Destroy    => Destroy(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Create(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MutexBuiltin.Create, 0, args.Count);
        var id = Interlocked.Increment(ref _nextId);
        _mutexes[id] = new SemaphoreSlim(1, 1);
        return Value.CreateInteger(id);
    }

    private static Value Acquire(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MutexBuiltin.Acquire, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, MutexBuiltin.Acquire, 0, args[0]);

        var id = args[0].GetInteger();
        if (!_mutexes.TryGetValue(id, out var sem))
            throw new InvalidOperationError(token, $"No mutex with id {id}.");

        sem.Wait();
        return Value.Default;
    }

    private static Value TryAcquire(Token token, List<Value> args)
    {
        if (args.Count < 1 || args.Count > 2)
            throw new ParameterCountMismatchError(token, MutexBuiltin.TryAcquire, args.Count, [1, 2]);

        ParameterTypeMismatchError.ExpectInteger(token, MutexBuiltin.TryAcquire, 0, args[0]);
        var id      = args[0].GetInteger();
        int timeout = args.Count == 2 && args[1].IsInteger() ? (int)args[1].GetInteger() : 0;

        if (!_mutexes.TryGetValue(id, out var sem))
            throw new InvalidOperationError(token, $"No mutex with id {id}.");

        return Value.CreateBoolean(sem.Wait(timeout));
    }

    private static Value Release(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MutexBuiltin.Release, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, MutexBuiltin.Release, 0, args[0]);

        var id = args[0].GetInteger();
        if (!_mutexes.TryGetValue(id, out var sem))
            throw new InvalidOperationError(token, $"No mutex with id {id}.");

        try { sem.Release(); } catch (SemaphoreFullException) { }
        return Value.Default;
    }

    private static Value Destroy(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, MutexBuiltin.Destroy, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, MutexBuiltin.Destroy, 0, args[0]);

        var id = args[0].GetInteger();
        if (_mutexes.TryRemove(id, out var sem))
            sem.Dispose();

        return Value.Default;
    }
}
