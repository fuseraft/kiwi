using kiwi.Typing;

namespace kiwi.Runtime;

internal sealed class TaskState
{
    public long Id;
    public LambdaRef Lambda = null!;
    public List<Value> Args = [];
    public Value Result = Value.Default;
    public TaskStatus Status;
    public ManualResetEventSlim Completion = new(false);
    public Exception? Exception;
}