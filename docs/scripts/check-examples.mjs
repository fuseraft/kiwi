// Runs every ```kiwi example in the docs through the interpreter and fails if
// any of them errors.
//
// Fence meta controls how a block is checked:
//   ```kiwi            must run cleanly (exit code 0, nothing on stderr)
//   ```kiwi fails      must fail (it demonstrates an error)
//   ```kiwi stderr     must exit 0, but may write to stderr (e.g. logging)
//   ```kiwi nocheck    not run, only syntax-checked (network access, files, input)
//   ```kiwi noparse    neither run nor parsed (deliberately incomplete fragments)
//
// Usage: node scripts/check-examples.mjs [--kiwi path] [--verbose] [filter]
import { readdirSync, readFileSync, writeFileSync, mkdtempSync, rmSync } from 'node:fs';
import { join, relative } from 'node:path';
import { tmpdir } from 'node:os';
import { spawnSync } from 'node:child_process';
import { fileURLToPath } from 'node:url';

const docsRoot = fileURLToPath(new URL('../src/content/docs/', import.meta.url));
const repoRoot = fileURLToPath(new URL('../../', import.meta.url));

const args = process.argv.slice(2);
const verbose = args.includes('--verbose');
const kiwiIndex = args.indexOf('--kiwi');
const kiwi = kiwiIndex >= 0 ? args[kiwiIndex + 1] : join(repoRoot, 'bin', 'kiwi');
const filter = args.find((a, i) => !a.startsWith('--') && args[i - 1] !== '--kiwi');

function* walk(dir) {
  for (const entry of readdirSync(dir, { withFileTypes: true })) {
    const path = join(dir, entry.name);
    if (entry.isDirectory()) yield* walk(path);
    else if (/\.mdx?$/.test(entry.name)) yield path;
  }
}

function* examples(file) {
  const lines = readFileSync(file, 'utf8').split('\n');
  for (let i = 0; i < lines.length; i++) {
    const open = lines[i].match(/^(\s*)(`{3,}|~{3,})kiwi\b(.*)$/);
    if (!open) continue;
    const [, indent, fence, meta] = open;
    const start = i + 1;
    const body = [];
    while (++i < lines.length && !lines[i].startsWith(`${indent}${fence}`)) body.push(lines[i].slice(indent.length));
    const words = meta.trim().split(/\s+/);
    if (words.includes('noparse')) continue;
    yield {
      line: start,
      code: body.join('\n') + '\n',
      expectFailure: words.includes('fails'),
      syntaxOnly: words.includes('nocheck'),
      allowStderr: words.includes('stderr'),
    };
  }
}

const work = mkdtempSync(join(tmpdir(), 'kiwi-docs-'));
let total = 0;
const failures = [];

for (const file of walk(docsRoot)) {
  const name = relative(docsRoot, file);
  if (filter && !name.includes(filter)) continue;
  for (const ex of examples(file)) {
    total++;
    const script = join(work, `example_${total}.kiwi`);
    writeFileSync(script, ex.code);
    const argv = ex.syntaxOnly ? ['--check', script] : [script];
    const run = spawnSync(kiwi, argv, { cwd: work, encoding: 'utf8', timeout: 15000, input: '' });
    const failed = Boolean(run.error) || run.status !== 0 || (!ex.allowStderr && run.stderr.trim() !== '');
    const where = `${name}:${ex.line}${ex.syntaxOnly ? ' (syntax only)' : ''}`;
    if (failed !== ex.expectFailure) {
      const reason = ex.expectFailure
        ? 'expected this example to fail, but it ran cleanly'
        : (run.error?.message ?? (run.stderr.trim() || `exit code ${run.status}`));
      failures.push({ where, reason, code: ex.code });
    }
    if (verbose) {
      console.log(`\n--- ${where}${ex.expectFailure ? ' (fails)' : ''}`);
      console.log(ex.code.trimEnd());
      console.log('>>>');
      console.log((run.stdout + run.stderr).trimEnd());
    }
  }
}

rmSync(work, { recursive: true, force: true });

for (const f of failures) {
  console.error(`\n✗ ${f.where}\n  ${f.reason.split('\n').join('\n  ')}`);
}
console.log(`\n${total - failures.length}/${total} examples passed.`);
process.exit(failures.length ? 1 : 0);
