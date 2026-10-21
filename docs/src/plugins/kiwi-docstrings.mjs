// Extracts the public API of a Kiwi source file from its `/# ... #/` docstrings
// and renders it as Markdown. Used by the remark plugin that fills in the
// "API reference" section of each standard library page.
import { readFileSync } from 'node:fs';

const DECL = {
  package: /^(\s*)package\s+([A-Za-z_][\w:]*)/,
  struct: /^(\s*)(abstract\s+)?struct\s+([A-Za-z_]\w*)(?:\s*<\s*([A-Za-z_][\w:]*))?/,
  enum: /^(\s*)enum\s+([A-Za-z_]\w*)/,
  fn: /^(\s*)((?:(?:static|private|override|abstract)\s+)*)(?:fn|def)\s+([A-Za-z_]\w*|[-+*/%<>=!&|^~]+)\s*(\(.*)?$/,
  const: /^(\s*)const\s+([A-Z_][A-Z0-9_]*)\s*=\s*(.+)$/,
  packageVar: /^(\s*)([A-Z][A-Z0-9_]*)\s*=\s*(.+)$/,
};

/** Removes the indentation shared by every non-blank line. */
function dedent(text) {
  const lines = text.split('\n');
  const indents = lines.filter((l) => l.trim()).map((l) => l.match(/^\s*/)[0].length);
  const cut = indents.length ? Math.min(...indents) : 0;
  return lines.map((l) => l.slice(cut)).join('\n');
}

/** Splits a docstring body into a summary and its tags. */
function parseDocstring(lines) {
  const doc = { summary: '', params: [], returns: null, throws: [], examples: [], see: [], deprecated: null };
  // Strip the common indentation so examples keep their relative layout.
  const indents = lines.filter((l) => l.trim()).map((l) => l.match(/^\s*/)[0].length);
  const cut = indents.length ? Math.min(...indents) : 0;
  const body = lines.map((l) => l.slice(cut));

  let current = null;
  let inExample = false;
  let example = [];

  for (const line of body) {
    if (current === 'example') {
      // Examples are usually fenced with `---` lines, but some docstrings
      // simply list the example lines after the tag.
      if (!inExample && line.trim() !== '---' && !/^@\w+/.test(line)) {
        if (line.trim() || example.length) example.push(line);
        continue;
      }
      if (!inExample && example.length && /^@\w+/.test(line)) {
        doc.examples.push(example.join('\n').trimEnd());
        example = [];
      }
      if (line.trim() === '---') {
        if (inExample) {
          doc.examples.push(example.join('\n'));
          example = [];
          inExample = false;
          current = null;
        } else {
          inExample = true;
        }
        continue;
      }
      if (inExample) {
        example.push(line);
        continue;
      }
    }

    const tag = line.match(/^@{1,2}(\w+)\s*(.*)$/);
    if (tag) {
      const [, name, rest] = tag;
      current = name;
      switch (name) {
        case 'summary':
          doc.summary = rest.trim();
          break;
        case 'param': {
          // Usually `@param type name : description`; some docstrings use `@param name description`.
          const typed = rest.match(/^(\S+)\s+(\*?\w+)\s*:\s*(.*)$/);
          const untyped = rest.match(/^(\*?\w+)\s*(.*)$/);
          if (typed) doc.params.push({ type: typed[1], name: typed[2], desc: typed[3] });
          else if (untyped) doc.params.push({ type: 'any', name: untyped[1], desc: untyped[2] });
          break;
        }
        case 'return':
        case 'returns': {
          const m = rest.match(/^([^:]+?)\s*:\s*(.*)$/);
          // `@return type : description`, or just a description.
          doc.returns = m ? { type: m[1].trim(), desc: m[2] } : { type: '', desc: rest.trim() };
          current = 'return';
          break;
        }
        case 'throws': {
          const m = rest.match(/^([^:]+?)\s*:\s*(.*)$/);
          doc.throws.push(m ? { type: m[1].trim(), desc: m[2] } : { type: '', desc: rest.trim() });
          break;
        }
        case 'see':
          doc.see.push(rest.trim());
          break;
        case 'deprecated':
          doc.deprecated = rest.replace(/^:\s*/, '').trim() || 'Deprecated.';
          break;
        case 'example':
          inExample = false;
          example = [];
          break;
        default:
          break;
      }
      continue;
    }

    // Continuation lines extend the most recent tag.
    const text = line.trim();
    if (!text) continue;
    if (current === null || current === 'summary') {
      doc.summary = doc.summary ? `${doc.summary} ${text}` : text;
      current = 'summary';
    } else if (current === 'param' && doc.params.length) {
      doc.params.at(-1).desc += ` ${text}`;
    } else if (current === 'return' && doc.returns) {
      doc.returns.desc += ` ${text}`;
    } else if (current === 'throws' && doc.throws.length) {
      doc.throws.at(-1).desc += ` ${text}`;
    }
  }
  if (example.length) doc.examples.push(example.join('\n').trimEnd());
  doc.examples = doc.examples.map(dedent);
  return doc;
}

/** Reads the parameter list and return type from a `fn` line (which may span lines). */
function readSignature(lines, start, tail) {
  let text = tail ?? '';
  let i = start;
  let depth = 0;
  const balanced = (s) => {
    depth = 0;
    let inStr = null;
    for (const ch of s) {
      if (inStr) {
        if (ch === inStr) inStr = null;
        continue;
      }
      if (ch === '"' || ch === "'") inStr = ch;
      else if (ch === '(' || ch === '[' || ch === '{') depth++;
      else if (ch === ')' || ch === ']' || ch === '}') depth--;
    }
    return depth <= 0;
  };
  while (text && !balanced(text) && i + 1 < lines.length) {
    i++;
    text += ' ' + lines[i].trim();
  }
  if (!text.startsWith('(')) return { params: '', returns: '' };

  // Find the matching close paren.
  let d = 0;
  let inStr = null;
  let close = -1;
  for (let k = 0; k < text.length; k++) {
    const ch = text[k];
    if (inStr) {
      if (ch === inStr) inStr = null;
      continue;
    }
    if (ch === '"' || ch === "'") inStr = ch;
    else if (ch === '(' || ch === '[' || ch === '{') d++;
    else if (ch === ')' || ch === ']' || ch === '}') {
      d--;
      if (d === 0) {
        close = k;
        break;
      }
    }
  }
  const params = text.slice(1, close).trim();
  const rest = text.slice(close + 1);
  const ret = rest.match(/^\s*:\s*([\w|?:]+)/);
  return { params, returns: ret ? ret[1] : '' };
}

/** Parses a Kiwi source file into packages, structs, functions and constants. */
export function parseKiwiSource(source) {
  const lines = source.replace(/\r\n/g, '\n').split('\n');
  const root = { kind: 'file', name: '', items: [] };
  const stack = [{ node: root, indent: -1 }];
  let pendingDoc = null;
  let region = null;

  const parentFor = (indent) => {
    while (stack.length > 1 && stack.at(-1).indent >= indent) stack.pop();
    return stack.at(-1).node;
  };

  for (let i = 0; i < lines.length; i++) {
    const line = lines[i];
    const trimmed = line.trim();

    if (trimmed.startsWith('/#')) {
      const docLines = [];
      let first = trimmed.slice(2);
      if (first.includes('#/')) {
        docLines.push(first.slice(0, first.indexOf('#/')));
      } else {
        if (first.trim()) docLines.push(first);
        while (++i < lines.length && !lines[i].includes('#/')) docLines.push(lines[i]);
        const last = lines[i] ?? '';
        const before = last.slice(0, last.indexOf('#/'));
        if (before.trim()) docLines.push(before);
      }
      pendingDoc = parseDocstring(docLines);
      continue;
    }
    if (!trimmed) continue;
    if (trimmed.startsWith('#region')) {
      region = trimmed.slice('#region'.length).trim() || null;
      continue;
    }
    if (trimmed.startsWith('#endregion')) {
      region = null;
      continue;
    }
    if (trimmed.startsWith('#')) continue;

    let m;
    if ((m = line.match(DECL.package))) {
      const indent = m[1].length;
      const parent = parentFor(indent);
      const qualified = parent.kind === 'package' ? `${parent.name}::${m[2]}` : m[2];
      const node = { kind: 'package', name: qualified, doc: pendingDoc, items: [] };
      parent.items.push(node);
      stack.push({ node, indent });
    } else if ((m = line.match(DECL.struct))) {
      const indent = m[1].length;
      const parent = parentFor(indent);
      const node = { kind: 'struct', name: m[3], abstract: !!m[2], base: m[4] ?? null, doc: pendingDoc, items: [] };
      parent.items.push(node);
      stack.push({ node, indent });
    } else if ((m = line.match(DECL.enum))) {
      const indent = m[1].length;
      const parent = parentFor(indent);
      const members = [];
      while (++i < lines.length && lines[i].trim() !== 'end') {
        const t = lines[i].trim().replace(/#.*$/, '').trim();
        if (t) members.push(t);
      }
      parent.items.push({ kind: 'enum', name: m[2], doc: pendingDoc, members });
    } else if ((m = line.match(DECL.fn))) {
      const indent = m[1].length;
      const parent = parentFor(indent);
      const modifiers = m[2].trim().split(/\s+/).filter(Boolean);
      const name = m[3];
      const sig = readSignature(lines, i, m[4]);
      const node = {
        kind: 'fn',
        name,
        static: modifiers.includes('static'),
        private: modifiers.includes('private') || (name.startsWith('_') && !/^__\w+__$/.test(name)),
        abstract: modifiers.includes('abstract'),
        params: sig.params,
        returns: sig.returns,
        doc: pendingDoc,
        region,
      };
      parent.items.push(node);
      // Anything nested inside a function body is not part of the API.
      stack.push({ node: { kind: 'fn-body', items: [] }, indent });
    } else if ((m = line.match(DECL.packageVar)) && parentFor(m[1].length).kind === 'package') {
      // Upper-case package variables (e.g. `SIGINT = "SIGINT"`) are documented like constants.
      parentFor(m[1].length).items.push({ kind: 'const', name: m[2], value: m[3].replace(/\s+#.*$/, '').trim(), doc: pendingDoc });
    } else if ((m = line.match(DECL.const))) {
      const indent = m[1].length;
      const parent = parentFor(indent);
      parent.items.push({ kind: 'const', name: m[2], value: m[3].replace(/\s+#.*$/, '').trim(), doc: pendingDoc });
    }
    pendingDoc = null;
  }
  return root;
}

const esc = (s) => String(s ?? '').replace(/\|/g, '\\|');

function splitParams(params) {
  const out = [];
  let depth = 0;
  let inStr = null;
  let cur = '';
  for (const ch of params) {
    if (inStr) {
      cur += ch;
      if (ch === inStr) inStr = null;
      continue;
    }
    if (ch === '"' || ch === "'") inStr = ch;
    if ('([{'.includes(ch)) depth++;
    if (')]}'.includes(ch)) depth--;
    if (ch === ',' && depth === 0) {
      out.push(cur.trim());
      cur = '';
    } else cur += ch;
  }
  if (cur.trim()) out.push(cur.trim());
  return out.map((p) => {
    const m = p.match(/^(\*?\w+)\s*(?::\s*([^=]+?))?\s*(?:=\s*(.+))?$/);
    return m ? { name: m[1], type: m[2]?.trim() ?? '', default: m[3]?.trim() } : { name: p, type: '' };
  });
}

function renderFn(fn, owner, level) {
  const out = [];
  const h = '#'.repeat(level);
  const doc = fn.doc ?? parseDocstring([]);
  let callName;
  if (owner.kind === 'struct') callName = fn.static || fn.name === 'new' ? `${owner.name}.${fn.name}` : `.${fn.name}`;
  else if (owner.kind === 'package') callName = `${owner.name}::${fn.name}`;
  else callName = fn.name;
  if (/^[^\w]/.test(fn.name)) callName = `operator ${fn.name}`;

  out.push(`${h} \`${callName}\``);
  out.push('');
  const params = fn.params ? splitParams(fn.params) : [];
  const shown = params.map((p) => `${p.name}${p.type ? `: ${p.type}` : ''}${p.default !== undefined ? ` = ${p.default}` : ''}`).join(', ');
  const prefix = owner.kind === 'struct' ? (fn.static || fn.name === 'new' ? `${owner.name}.` : '') : owner.kind === 'package' ? `${owner.name}::` : '';
  const sigName = /^[^\w]/.test(fn.name) ? `fn ${fn.name}` : `${prefix}${fn.name}`;
  out.push('```kiwi nocheck');
  out.push(`${fn.static ? 'static ' : ''}${sigName}(${shown})${fn.returns ? `: ${fn.returns}` : ''}`);
  out.push('```');
  out.push('');
  if (doc.deprecated) out.push(`:::caution[Deprecated]\n${doc.deprecated}\n:::\n`);
  if (doc.summary) out.push(doc.summary, '');

  const documented = new Map(doc.params.map((p) => [p.name.replace(/^\*/, ''), p]));
  if (params.length) {
    out.push('| Parameter | Type | Default | Description |');
    out.push('| :-- | :-- | :-- | :-- |');
    for (const p of params) {
      const d = documented.get(p.name.replace(/^\*/, ''));
      const type = p.type || (d && d.type !== 'any' ? d.type : d?.type) || 'any';
      out.push(`| \`${esc(p.name)}\` | \`${esc(type)}\` | ${p.default !== undefined ? `\`${esc(p.default)}\`` : ''} | ${esc(d?.desc ?? '')} |`);
    }
    out.push('');
  }
  if (doc.returns) {
    const type = doc.returns.type || fn.returns;
    const typeText = type ? `\`${type}\`` : '';
    const sep = typeText && doc.returns.desc ? ' — ' : '';
    out.push(`**Returns** ${typeText}${sep}${doc.returns.desc}`, '');
  } else if (fn.returns) {
    out.push(`**Returns** \`${fn.returns}\``, '');
  }
  for (const t of doc.throws) {
    out.push(`**Throws** ${t.type ? `\`${t.type}\`` : ''}${t.desc ? `${t.type ? ' — ' : ''}${t.desc}` : ''}`, '');
  }
  for (const ex of doc.examples) {
    out.push('```kiwi nocheck', ex.replace(/\s+$/, ''), '```', '');
  }
  return out;
}

// A later definition with the same name replaces an earlier one at runtime,
// so only the last one is documented.
function lastDefinitions(items) {
  const seen = new Map();
  items.forEach((x, i) => seen.set(`${x.kind}:${x.static ? 's' : ''}:${x.name}`, i));
  return items.filter((x, i) => seen.get(`${x.kind}:${x.static ? 's' : ''}:${x.name}`) === i);
}

function isPublic(item) {
  if (item.kind === 'fn') return !item.private;
  return !item.name?.startsWith('_');
}

function renderStruct(struct, level) {
  const out = [];
  const h = '#'.repeat(level);
  out.push(`${h} \`${struct.name}\` ${struct.abstract ? 'abstract struct' : 'struct'}`, '');
  if (struct.base) out.push(`Inherits from \`${struct.base}\`.`, '');
  if (struct.doc?.summary) out.push(struct.doc.summary, '');
  for (const ex of struct.doc?.examples ?? []) out.push('```kiwi nocheck', ex, '```', '');
  const fns = lastDefinitions(struct.items).filter((x) => x.kind === 'fn' && isPublic(x));
  const ctor = fns.filter((f) => f.name === 'new');
  const statics = fns.filter((f) => f.static && f.name !== 'new');
  const methods = fns.filter((f) => !f.static && f.name !== 'new');
  for (const f of [...ctor, ...statics, ...methods]) out.push(...renderFn(f, struct, level + 1));
  return out;
}

function renderPackage(pkg, level) {
  const out = [];
  const items = lastDefinitions(pkg.items);
  const consts = items.filter((x) => x.kind === 'const');
  const fns = items.filter((x) => x.kind === 'fn' && isPublic(x));
  const structs = items.filter((x) => x.kind === 'struct' && isPublic(x));
  const enums = pkg.items.filter((x) => x.kind === 'enum');
  const h = '#'.repeat(level);

  if (consts.length) {
    out.push(`${h} Constants`, '');
    out.push('| Name | Value | Description |', '| :-- | :-- | :-- |');
    for (const c of consts) out.push(`| \`${pkg.name}::${c.name}\` | \`${esc(c.value)}\` | ${esc(c.doc?.summary ?? '')} |`);
    out.push('');
  }
  if (fns.length) {
    out.push(`${h} Functions`, '');
    for (const f of fns) out.push(...renderFn(f, pkg, level + 1));
  }
  for (const e of enums) {
    out.push(`${h} \`${e.name}\` enum`, '');
    if (e.doc?.summary) out.push(e.doc.summary, '');
    out.push('```kiwi nocheck', `enum ${e.name}`, ...e.members.map((m) => `  ${m}`), 'end', '```', '');
  }
  for (const s of structs) out.push(...renderStruct(s, level));
  for (const sub of pkg.items.filter((x) => x.kind === 'package')) {
    out.push(`${h} \`${sub.name}\``, '');
    if (sub.doc?.summary) out.push(sub.doc.summary, '');
    out.push(...renderPackage(sub, level + 1));
  }
  return out;
}

/** Renders the public API of a Kiwi source file as Markdown, starting at `level`. */
export function renderApi(file, level = 2) {
  const tree = parseKiwiSource(readFileSync(file, 'utf8'));
  const out = [];
  const packages = tree.items.filter((x) => x.kind === 'package');
  const topStructs = lastDefinitions(tree.items).filter((x) => x.kind === 'struct' && isPublic(x));
  const topFns = tree.items.filter((x) => x.kind === 'fn' && isPublic(x));

  if (packages.length === 1) {
    out.push(...renderPackage(packages[0], level));
  } else {
    for (const pkg of packages) {
      out.push(`${'#'.repeat(level)} \`${pkg.name}\` package`, '');
      if (pkg.doc?.summary) out.push(pkg.doc.summary, '');
      out.push(...renderPackage(pkg, level + 1));
    }
  }
  if (topFns.length) {
    out.push(`${'#'.repeat(level)} Functions`, '');
    for (const f of topFns) out.push(...renderFn(f, { kind: 'file' }, level + 1));
  }
  for (const s of topStructs) out.push(...renderStruct(s, level));
  return out.join('\n');
}

/** Returns the package summary of a Kiwi source file (the docstring above `package`). */
export function packageSummary(file) {
  const tree = parseKiwiSource(readFileSync(file, 'utf8'));
  const pkg = tree.items.find((x) => x.kind === 'package');
  return pkg?.doc?.summary ?? '';
}
