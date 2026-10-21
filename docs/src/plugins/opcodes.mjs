// Renders the VM instruction set from the doc comments in src/VM/Opcode.cs.
import { readFileSync } from 'node:fs';

// Opcodes declared in a group on one line share no doc comment of their own.
const fallback = {
  Pop: 'Discard the top value.',
  Dup: 'Duplicate the top value.',
  Add: 'pop right, pop left → push left + right',
  Sub: 'pop right, pop left → push left - right',
  Mul: 'pop right, pop left → push left * right',
  Div: 'pop right, pop left → push left / right (float)',
  IntDiv: 'pop right, pop left → push left // right (floor)',
  Mod: 'pop right, pop left → push left % right',
  Pow: 'pop right, pop left → push left ** right',
  BAnd: 'pop right, pop left → push left & right',
  BOr: 'pop right, pop left → push left | right',
  BXor: 'pop right, pop left → push left ^ right',
  BNot: 'pop value → push ~value',
  BLSh: 'pop right, pop left → push left << right',
  BRSh: 'pop right, pop left → push left >> right',
  BURSh: 'pop right, pop left → push left >>> right',
  Eq: 'pop right, pop left → push left == right',
  NEq: 'pop right, pop left → push left != right',
  Lt: 'pop right, pop left → push left < right',
  LtE: 'pop right, pop left → push left <= right',
  Gt: 'pop right, pop left → push left > right',
  GtE: 'pop right, pop left → push left >= right',
  Nop: 'Do nothing.',
  Halt: 'Stop executing the chunk.',
};

const escapeCell = (s) => s.replace(/\|/g, '\\|').replace(/<c>(.*?)<\/c>/g, '`$1`').replace(/&lt;/g, '<').replace(/&gt;/g, '>');

/** Parses Opcode.cs into sections of { name, description } entries. */
export function parseOpcodes(source) {
  const sections = [];
  let section = null;
  let summary = [];
  let inSummary = false;

  for (const raw of source.split('\n')) {
    const line = raw.trim();
    const heading = line.match(/^\/\/\s*--\s*(.+?)\s*-+\s*$/);
    if (heading) {
      section = { title: heading[1], opcodes: [] };
      sections.push(section);
      summary = [];
      continue;
    }
    if (!section) continue;
    if (line.startsWith('/// <summary>')) {
      inSummary = true;
      summary = [];
      continue;
    }
    if (line.startsWith('/// </summary>')) {
      inSummary = false;
      continue;
    }
    if (line.startsWith('///')) {
      if (inSummary) summary.push(line.replace(/^\/\/\/\s?/, ''));
      continue;
    }
    const members = line.match(/^([A-Z]\w*(?:\s*,\s*[A-Z]\w*)*)\s*,?\s*$/);
    if (members) {
      const names = members[1].split(',').map((n) => n.trim()).filter(Boolean);
      const description = summary.join(' ').replace(/\s+/g, ' ').trim();
      for (const name of names) {
        const own = names.length === 1 ? description : '';
        section.opcodes.push({ name, description: own || fallback[name] || '' });
      }
      summary = [];
    }
  }
  return sections.filter((s) => s.opcodes.length);
}

/** Renders the instruction set as Markdown tables, one per section. */
export function renderOpcodes(file, level = 3) {
  const sections = parseOpcodes(readFileSync(file, 'utf8'));
  const out = [];
  for (const s of sections) {
    out.push(`${'#'.repeat(level)} ${s.title}`, '', '| Opcode | Behavior |', '| :-- | :-- |');
    for (const op of s.opcodes) out.push(`| \`${op.name}\` | ${escapeCell(op.description)} |`);
    out.push('');
  }
  return out.join('\n');
}
