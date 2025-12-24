using System.Net.Sockets;

namespace kiwi.Runtime;

internal sealed class SocketState
{
    // Public init is fine as we control creation.
    public long Id { get; init; }
    public required Socket Socket { get; init; }
    
    public SocketRole Role { get; init; }
    
    // Channels are created at construction time and are never null.
    public Channel ReadChannel { get; init; } = Channel.Create(16);
    public Channel WriteChannel { get; init; } = Channel.Create(16);
    
    // Only listeners have this, so nullable is correct here.
    public Channel? AcceptChannel { get; init; }
}

internal enum SocketRole
{
    Listener,
    Client
}