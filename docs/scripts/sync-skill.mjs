// Regenerates the reference files of the kiwi-expert agent skill
// (.agents/skills/kiwi-expert/references/) from the documentation pages, with
// the generated API sections expanded and site links rewritten to file paths.
//
// Usage: node scripts/sync-skill.mjs
import { readdirSync, readFileSync, writeFileSync, mkdirSync, rmSync } from 'node:fs';
import { join, relative, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';
import { renderApi } from '../src/plugins/kiwi-docstrings.mjs';
import { renderOpcodes } from '../src/plugins/opcodes.mjs';

const docsRoot = fileURLToPath(new URL('../src/content/docs/', import.meta.url));
const repoRoot = fileURLToPath(new URL('../../', import.meta.url));
const outRoot = join(repoRoot, '.agents/skills/kiwi-expert/references');

// Site section -> reference directory.
const sections = {
  'getting-started': 'guide',
  language: 'language',
  tools: 'tools',
  reference: 'reference',
  stdlib: 'lib',
  internals: 'internals',
};

function* walk(dir) {
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const path = join(dir, entry.name);
    if (entry.isDirectory()) yield* walk(path);
    else if (entry.name.endsWith('.md')) yield path;
  }
}

/** Maps a docs page path like `language/functions.md` to its reference path. */
function targetFor(page) {
  const [section, ...rest] = page.split('/');
  const dir = sections[section];
  if (!dir) return null;
  const name = rest.join('/').replace(/\.md$/, '');
  return `${dir}/${name === 'index' ? 'README' : name}.md`;
}

/** Rewrites `/kiwi/section/page/#anchor` links into relative reference paths. */
function rewriteLinks(markdown, fromFile) {
  return markdown.replace(/\]\(\/kiwi\/([^)#]*?)\/?(#[^)]*)?\)/g, (match, path, anchor = '') => {
    const page = path === 'stdlib' ? 'stdlib/index.md' : `${path}.md`;
    const target = targetFor(page);
    if (!target) return `](https://fuseraft.ai/kiwi/${path}/${anchor})`;
    const rel = relative(dirname(fromFile), target) || target;
    return `](${rel}${anchor})`;
  });
}

function expandMarkers(markdown) {
  return markdown
    .replace(/^<!--\s*api:\s*(\w+)\s*(?:level=(\d))?\s*-->$/gm, (_, name, level) =>
      renderApi(join(repoRoot, 'lib', `${name}.kiwi`), Number(level ?? 2)))
    .replace(/^<!--\s*opcodes\s*-->$/gm, () => renderOpcodes(join(repoRoot, 'src/VM/Opcode.cs')));
}

function convert(text, target) {
  const fm = text.match(/^---\n([\s\S]*?)\n---\n/);
  const meta = fm ? fm[1] : '';
  const title = (meta.match(/^title:\s*"?(.*?)"?$/m) ?? [])[1] ?? '';
  const description = (meta.match(/^description:\s*"?(.*?)"?$/m) ?? [])[1] ?? '';
  let body = fm ? text.slice(fm[0].length) : text;
  body = expandMarkers(body);
  // Fence modifiers only matter to the docs example checker.
  body = body.replace(/^(\s*```kiwi)\s+(?:nocheck|noparse|fails|stderr)\s*$/gm, '$1');
  body = rewriteLinks(body, target);
  const header = `# ${title}\n\n` + (description ? `> ${description}\n\n` : '');
  return `${header}${body.trim()}\n`;
}

rmSync(outRoot, { recursive: true, force: true });
let count = 0;
for (const file of walk(docsRoot)) {
  const page = relative(docsRoot, file);
  const target = targetFor(page);
  if (!target) continue;
  const out = join(outRoot, target);
  mkdirSync(dirname(out), { recursive: true });
  writeFileSync(out, convert(readFileSync(file, 'utf8'), target));
  count++;
}
console.log(`Wrote ${count} reference files to ${relative(repoRoot, outRoot)}/`);
