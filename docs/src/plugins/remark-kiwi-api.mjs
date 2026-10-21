// Replaces generated-content markers in a page with Markdown built from the
// interpreter sources, so reference material can't drift from the code:
//
//   <!-- api: <file> -->   API reference from the docstrings in lib/<file>.kiwi
//   <!-- opcodes -->       VM instruction set from src/VM/Opcode.cs
import { fileURLToPath } from 'node:url';
import { unified } from 'unified';
import remarkParse from 'remark-parse';
import remarkGfm from 'remark-gfm';
import { renderApi } from './kiwi-docstrings.mjs';
import { renderOpcodes } from './opcodes.mjs';

const repoRoot = fileURLToPath(new URL('../../../', import.meta.url));
const apiMarker = /^<!--\s*api:\s*(\w+)\s*(?:level=(\d))?\s*-->$/;
const opcodesMarker = /^<!--\s*opcodes\s*-->$/;

function generate(comment) {
  const api = comment.match(apiMarker);
  if (api) return renderApi(`${repoRoot}lib/${api[1]}.kiwi`, Number(api[2] ?? 2));
  if (opcodesMarker.test(comment)) return renderOpcodes(`${repoRoot}src/VM/Opcode.cs`);
  return null;
}

export default function remarkKiwiApi() {
  const parser = unified().use(remarkParse).use(remarkGfm);
  return (tree) => {
    for (let i = 0; i < tree.children.length; i++) {
      const node = tree.children[i];
      if (node.type !== 'html') continue;
      const md = generate(node.value.trim());
      if (md === null) continue;
      const generated = parser.parse(md);
      tree.children.splice(i, 1, ...generated.children);
      i += generated.children.length - 1;
    }
  };
}
