namespace kiwi.Parsing.Keyword;

public static class ChannelBuiltin
{
    public const string Create   = "__chan_create__";
    public const string Send     = "__chan_send__";
    public const string Recv     = "__chan_recv__";
    public const string TryRecv  = "__chan_tryrecv__";
    public const string Close    = "__chan_close__";

    private static readonly IReadOnlyDictionary<string, TokenName> _map 
        = new Dictionary<string, TokenName>
        {
            { Create,   TokenName.Builtin_Channel_Create },
            { Send,     TokenName.Builtin_Channel_Send },
            { Recv,     TokenName.Builtin_Channel_Recv },
            { TryRecv,  TokenName.Builtin_Channel_TryRecv },
            { Close,    TokenName.Builtin_Channel_Close },
        };

    private static readonly IReadOnlySet<TokenName> _names = Map.Values.ToHashSet();

    public static IReadOnlyDictionary<string, TokenName> Map => _map;
    
    public static bool IsBuiltin(TokenName name) => _names.Contains(name);
}