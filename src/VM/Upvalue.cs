using kiwi.Typing;

namespace kiwi.VM;

/// <summary>
/// A heap-allocated cell that holds a variable captured by a closure.
///
/// While the enclosing function is still on the call stack (open state), the
/// upvalue points directly into the shared VM value stack so all readers and
/// writers see the same location.  When the enclosing frame is torn down
/// (closed state), the current stack value is copied into this object and the
/// stack reference is released.
///
/// Multiple closures can share one Upvalue object for the same variable.
/// </summary>
public sealed class Upvalue(Value[] stack, int absoluteSlot)
{
    private Value[]? _stack = stack;   // non-null while open
    private int _slot = absoluteSlot;    // absolute stack index (open state only)
    private Value    _closed;  // value copied here when closed

    /// <summary>
    /// Linked-list pointer: the VM keeps a singly-linked list of all open upvalues
    /// ordered by descending absolute slot so it can close them efficiently.
    /// </summary>
    public Upvalue? Next { get; set; }

    public int  AbsoluteSlot => _slot;
    public bool IsOpen       => _stack != null;

    public Value Get() => _stack != null ? _stack[_slot] : _closed;

    public void Set(Value v)
    {
        if (_stack != null) _stack[_slot] = v;
        else                _closed       = v;
    }

    /// <summary>
    /// Close the upvalue: snapshot the stack value into the heap cell and
    /// detach from the stack.  Must be called before the owning frame is popped.
    /// </summary>
    public void Close()
    {
        _closed = _stack![_slot];
        _stack  = null;
    }
}
