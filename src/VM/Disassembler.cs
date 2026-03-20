using System.Text;

namespace kiwi.VM;

/// <summary>
/// Produces a human-readable disassembly of a Chunk's bytecode.
/// Useful for debugging the compiler and VM.
/// </summary>
public static class Disassembler
{
    public static string Disassemble(Chunk chunk, int depth = 0)
    {
        var sb  = new StringBuilder();
        var pad = new string(' ', depth * 2);

        sb.AppendLine($"{pad}=== {chunk.Name} (arity={chunk.Arity}, locals={chunk.LocalCount}, upvalues={chunk.Upvalues.Count}) ===");

        if (chunk.ParamNames.Count > 0)
            sb.AppendLine($"{pad}  params: {string.Join(", ", chunk.ParamNames)}");

        for (int i = 0; i < chunk.Code.Count; i++)
        {
            var instr = chunk.Code[i];
            int line  = chunk.GetLine(i);
            sb.Append($"{pad}  {i,4}  [{line,4}]  {instr.Op,-18}");

            switch (instr.Op)
            {
                case Opcode.Const:
                    var cv = chunk.Constants.Count > instr.A ? chunk.Constants[instr.A] : null;
                    sb.Append($" {instr.A,4}   ({cv})");
                    break;

                case Opcode.LoadLocal:
                case Opcode.StoreLocal:
                    sb.Append($" slot={instr.A}");
                    break;

                case Opcode.LoadGlobal:
                case Opcode.StoreGlobal:
                {
                    var nm = chunk.Names.Count > instr.A ? chunk.Names[instr.A] : "?";
                    sb.Append($" \"{nm}\"");
                    break;
                }

                case Opcode.LoadUpvalue:
                case Opcode.StoreUpvalue:
                    sb.Append($" uv={instr.A}");
                    break;

                case Opcode.Jump:
                case Opcode.JumpF:
                case Opcode.JumpT:
                case Opcode.JumpAnd:
                case Opcode.JumpOr:
                    sb.Append($" -> {instr.A}");
                    break;

                case Opcode.ForIterNext:
                    sb.Append($" done->{instr.A}  vars={instr.B}");
                    break;

                case Opcode.Call:
                case Opcode.CallSplat:
                    sb.Append($" argc={instr.A}");
                    break;

                case Opcode.CallMethod:
                case Opcode.MethodCallSplat:
                {
                    var mn = chunk.Names.Count > instr.B ? chunk.Names[instr.B] : "?";
                    sb.Append($" .{mn}  argc={instr.A}");
                    break;
                }

                case Opcode.CallNamed:
                {
                    int nsIdx = instr.B;
                    var ns = chunk.ArgNameSets.Count > nsIdx
                        ? string.Join(", ", chunk.ArgNameSets[nsIdx].Select(n => string.IsNullOrEmpty(n) ? "_" : n))
                        : "?";
                    sb.Append($" argc={instr.A}  names=[{ns}]");
                    break;
                }

                case Opcode.CallMethodNamed:
                {
                    int methodNameIdx = instr.B & 0xFFFF;
                    int nsIdx         = (instr.B >> 16) & 0xFFFF;
                    var mn = chunk.Names.Count > methodNameIdx ? chunk.Names[methodNameIdx] : "?";
                    var ns = chunk.ArgNameSets.Count > nsIdx
                        ? string.Join(", ", chunk.ArgNameSets[nsIdx].Select(n => string.IsNullOrEmpty(n) ? "_" : n))
                        : "?";
                    sb.Append($" .{mn}  argc={instr.A}  names=[{ns}]");
                    break;
                }

                case Opcode.DefFunc:
                {
                    var fn = chunk.Names.Count > instr.B ? chunk.Names[instr.B] : "?";
                    sb.Append($" sub[{instr.A}]  name=\"{fn}\"");
                    break;
                }

                case Opcode.MakeClosure:
                    sb.Append($" sub[{instr.A}]  upvalues={instr.B}");
                    break;

                case Opcode.BuildList:
                case Opcode.BuildHashmap:
                    sb.Append($" count={instr.A}");
                    break;

                case Opcode.Interpolate:
                    sb.Append($" parts={instr.A}");
                    break;

                case Opcode.Print:
                {
                    bool nl  = (instr.A & 1) != 0;
                    bool err = (instr.A & 2) != 0;
                    sb.Append($" newline={nl}  stderr={err}");
                    break;
                }

                case Opcode.GetMember:
                case Opcode.SetMember:
                case Opcode.LoadSelfAttr:
                case Opcode.StoreSelfAttr:
                case Opcode.LoadStaticAttr:
                case Opcode.StoreStaticAttr:
                {
                    var nm = chunk.Names.Count > instr.A ? chunk.Names[instr.A] : "?";
                    sb.Append($" \"{nm}\"");
                    break;
                }

                case Opcode.NewObject:
                {
                    var nm = chunk.Names.Count > instr.B ? chunk.Names[instr.B] : "?";
                    sb.Append($" \"{nm}\"  argc={instr.A}");
                    break;
                }

                case Opcode.EventOn:
                case Opcode.EventOnce:
                    sb.Append($" priority={instr.A}");
                    break;

                case Opcode.EventOff:
                    sb.Append(instr.A == 1 ? " with-callback" : " all");
                    break;

                case Opcode.EventEmit:
                    sb.Append($" argc={instr.A}");
                    break;

                case Opcode.StructBegin:
                {
                    var sn   = chunk.Names.Count > instr.A ? chunk.Names[instr.A] : "?";
                    bool abs = (instr.B & 1) != 0;
                    int  br  = instr.B >> 1;
                    var  bn  = br > 0 && chunk.Names.Count > br - 1 ? chunk.Names[br - 1] : null;
                    sb.Append($" \"{sn}\"");
                    if (bn != null) sb.Append($"  base=\"{bn}\"");
                    if (abs)        sb.Append("  abstract");
                    break;
                }

                case Opcode.DefMethod:
                {
                    bool abs    = (instr.B & unchecked((int)0x80000000)) != 0;
                    bool stat   = (instr.B & 0x40000000) != 0;
                    int  ni     = instr.B & 0x3FFFFFFF;
                    var  mn     = chunk.Names.Count > ni ? chunk.Names[ni] : "?";
                    sb.Append($" sub[{instr.A}]  name=\"{mn}\"");
                    if (abs)  sb.Append("  abstract");
                    if (stat) sb.Append("  static");
                    break;
                }

                case Opcode.InitStructStatic:
                {
                    var vn = chunk.Names.Count > instr.A ? chunk.Names[instr.A] : "?";
                    sb.Append($" \"{vn}\"");
                    break;
                }

                case Opcode.PackageBegin:
                {
                    var pn = chunk.Names.Count > instr.A ? chunk.Names[instr.A] : "?";
                    sb.Append($" \"{pn}\"");
                    break;
                }

                case Opcode.EnumBegin:
                case Opcode.EnumEnd:
                {
                    var en = chunk.Names.Count > instr.A ? chunk.Names[instr.A] : "?";
                    sb.Append($" \"{en}\"");
                    break;
                }

                case Opcode.DefEnumMember:
                {
                    var mn = chunk.Names.Count > instr.A ? chunk.Names[instr.A] : "?";
                    sb.Append($" \"@@{mn}\"");
                    break;
                }

                case Opcode.InterpFallback:
                {
                    var nodeType = chunk.NodePool.Count > instr.A
                        ? chunk.NodePool[instr.A].Type.ToString() : "?";
                    sb.Append($" node[{instr.A}] ({nodeType})");
                    break;
                }

                case Opcode.CallBuiltin:
                {
                    var nodeType = chunk.NodePool.Count > instr.A
                        ? chunk.NodePool[instr.A].Type.ToString() : "?";
                    sb.Append($" node[{instr.A}] ({nodeType})  argc={instr.B}");
                    break;
                }

                case Opcode.Export:
                case Opcode.Eval:
                case Opcode.Include:
                {
                    sb.Append($" node[{instr.A}]");
                    break;
                }

                case Opcode.CloseUpvalue:
                    sb.Append($" from_slot={instr.A}");
                    break;

                case Opcode.SliceGet:
                case Opcode.SliceSet:
                    sb.Append($" flags=0x{instr.A:X}");
                    break;

                default:
                    if (instr.A != 0 || instr.B != 0)
                        sb.Append($" a={instr.A}  b={instr.B}");
                    break;
            }

            sb.AppendLine();
        }

        // Recursively disassemble sub-chunks
        for (int i = 0; i < chunk.SubChunks.Count; i++)
        {
            sb.AppendLine();
            sb.Append(Disassemble(chunk.SubChunks[i], depth + 1));
        }

        return sb.ToString();
    }

    /// <summary>
    /// Print disassembly to stdout.
    /// </summary>
    public static void Dump(Chunk chunk) => Console.Write(Disassemble(chunk));
}
