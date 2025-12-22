using System.Collections.Concurrent;
using kiwi.Parsing;
using kiwi.Tracing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime;

/// <summary>
/// Manages lightweight tasks (green threads on .NET thread pool).
/// </summary>
public sealed class TaskManager
{
    private long _nextId = 1;

    // Stores active tasks, maps id to state (removed on completion in Await).
    private readonly ConcurrentDictionary<long, TaskState> _tasks = new();

    // Reusable TaskState objects: grow-only, lock-free pooling.
    private readonly ConcurrentBag<TaskState> _stateBag = [];

    public static TaskManager Instance { get; } = new();

    private TaskManager() { }

    public Channel CreateChannel(int capacity = 0) => Channel.Create(capacity);

    private TaskState Rent() => _stateBag.TryTake(out var state) ? state : new TaskState();

    private void Return(TaskState state)
    {
        // clear references to prevent leaks across tasks
        state.Lambda = null!;
        state.Args.Clear();
        state.Result = Value.Default;
        state.Status = TaskStatus.Running; // ready for next spawn
        state.Exception = null;
        state.Completion.Reset();

        _stateBag.Add(state);
    }

    /// <summary>
    /// Spawns a new task by queuing the lambda on the thread pool.
    /// Returns the task ID.
    /// </summary>
    public long Spawn(KContext context, LambdaRef lambda, List<Value> args, Token token)
    {
        var state = Rent();
        state.Id = Interlocked.Increment(ref _nextId);
        state.Lambda = lambda;
        state.Args = args;
        state.Status = TaskStatus.Running;

        _tasks[state.Id] = state;

        // Fire-and-forget on thread pool
        ThreadPool.UnsafeQueueUserWorkItem(_ =>
        {
            try
            {
                // Each task gets its own isolated interpreter instance
                // TODO: globals copied from main
                var taskInterp = new Interpreter
                {
                    CurrentTaskId = state.Id,
                };

                taskInterp.SetContext(context);

                state.Result = taskInterp.InvokeEvent(token, state.Lambda, state.Args);
                state.Status = TaskStatus.Completed;
            }
            catch (Exception ex)
            {
                state.Exception = ex;
                state.Status = TaskStatus.Faulted;
            }
            finally
            {
                state.Completion.Set();
                // Do NOT return to pool here, Await will do it after result delivery
            }
        }, null);

        return state.Id;
    }

    /// <summary>
    /// Blocks the current fiber until the task completes.
    /// Returns the task result. Removes task from active map.
    /// </summary>
    public Value Await(long id, Token token)
    {
        if (!_tasks.TryRemove(id, out var state))
        {
            throw new InvalidOperationError(token, $"Task {id} not found or already completed.");
        }

        // Block only this fiber/thread
        state.Completion.Wait();

        if (state.Exception != null)
        {
            // WIP: log crash for diagnostics, then rethrow
            ErrorHandler.DumpCrashLog(state.Exception);
            throw state.Exception;
        }

        var result = state.Result;
        Return(state); // return to pool
        return result;
    }

    /// <summary>
    /// Simple blocking sleep in milliseconds – used by __task_sleep__ builtin
    /// </summary>
    public void Sleep(long ms)
    {
        if (ms > 0)
        {
            Thread.Sleep((int)Math.Min(ms, int.MaxValue));
        }
    }

    // Query helpers for __task_*__ builtins
    public Value Status(long id) => _tasks.TryGetValue(id, out var s)
            ? Value.CreateString(s.Status.ToString())
            : Value.CreateString("Unknown"); // TODO: cleanup these throughout the code

    public Value Result(long id) => _tasks.TryGetValue(id, out var s)
            ? s.Result
            : Value.Default;

    public List<Value> List()
    {
        var list = new List<Value>();
        foreach (var id in _tasks.Keys)
        {
            list.Add(Value.CreateInteger(id));
        }
        return list;
    }

    public Value Busy() => Value.CreateBoolean(!_tasks.IsEmpty);
}