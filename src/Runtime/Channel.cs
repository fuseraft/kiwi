using System.Threading.Channels;
using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime;

public sealed class Channel
{
    private readonly Channel<Value> _inner;
    private bool _closed;

    private Channel(int capacity)
    {
        if (capacity <= 0)
        {
            _inner = System.Threading.Channels.Channel.CreateUnbounded<Value>
            (
                new UnboundedChannelOptions
                {
                    SingleWriter = false,
                    SingleReader = false
                }
            );
        }
        else
        {
            _inner = System.Threading.Channels.Channel.CreateBounded<Value>
            (
                new BoundedChannelOptions(capacity)
                { 
                    FullMode = BoundedChannelFullMode.Wait, 
                    SingleWriter = false, 
                    SingleReader = false 
                }
            );
        }
    }

    public static Channel Create(int capacity = 0) => new(capacity);

    public void Send(Token token, Value value)
    {
        if (_closed)
        {
            throw new InvalidOperationError(token, "Send on a closed channel.");
        }

        // Try fast path first
        if (_inner.Writer.TryWrite(value))
        {
            return;
        }

        // Block current task until space
        _inner.Writer.WaitToWriteAsync().AsTask().Wait();
        _inner.Writer.TryWrite(value); // must succeed now
    }

    public Value Receive(Token token)
    {
        if (_inner.Reader.TryRead(out var value))
        {
            return value;
        }

        // Block until data or completion
        if (!_inner.Reader.WaitToReadAsync().AsTask().Result)
        {
            throw new InvalidOperationError(token, "Receive on a closed channel");
        }

        return _inner.Reader.ReadAsync().AsTask().Result;
    }

    public (bool Success, Value Value) TryReceive()
    {
        return _inner.Reader.TryRead(out var v) ? (true, v) : (false, Value.Default);
    }

    public void Close()
    {
        if (!_closed)
        {
            _closed = true;
            _inner.Writer.Complete();
        }
    }

    public bool Closed => _closed;
}