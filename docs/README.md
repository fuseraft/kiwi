# Kiwi documentation

The source of [fuseraft.ai/kiwi](https://fuseraft.ai/kiwi/), built with [Astro Starlight](https://starlight.astro.build/).

## Working on the docs

Requires Node.js 22 or later. Build Kiwi first (`./build.sh` in the repository root), because the example checker runs `../bin/kiwi`.

```sh
cd docs
npm install
npm run dev              # live preview at http://localhost:4321/kiwi/
npm run check:examples   # run every Kiwi example in the docs
npm run build            # build the site into dist/ and validate links
```

CI (`.github/workflows/docs.yml`) runs the example check and the build on every pull request, and deploys to GitHub Pages from `main`.

## Layout

| Path | Contents |
| :-- | :-- |
| `src/content/docs/` | The pages, one Markdown file each. The directory structure is the URL structure. |
| `src/content/docs/stdlib/` | One page per standard library package. |
| `astro.config.mjs` | Site configuration and the sidebar. |
| `src/grammars/kiwi.tmLanguage.json` | Kiwi syntax highlighting. |
| `src/plugins/` | Generators for reference material (see below). |
| `scripts/check-examples.mjs` | The example checker. |

New pages in `language/`, `tools/`, `reference/` and `getting-started/` must be added to the sidebar in `astro.config.mjs`. Standard library pages are picked up automatically.

## Examples are tested

Every ` ```kiwi ` block is run by `npm run check:examples`. It must exit with code 0 and write nothing to stderr. Write the expected output as a trailing comment (`println x   # 42`) and check it against the output shown by `node scripts/check-examples.mjs --verbose <page>`.

Add a word after `kiwi` on the fence line to change how a block is checked:

| Fence | Checked by |
| :-- | :-- |
| ` ```kiwi ` | Running it; it must succeed. |
| ` ```kiwi fails ` | Running it; it must fail. Use for examples of errors. |
| ` ```kiwi stderr ` | Running it; it must succeed but may write to stderr. |
| ` ```kiwi nocheck ` | `kiwi --check` only. Use for examples that need the network, files, input or a long-running process. |
| ` ```kiwi noparse ` | Nothing. Use only for deliberately incomplete fragments. |

Use ` ```text ` for program output and terminal sessions, and ` ```sh ` for shell commands.

## Generated reference material

Some content is generated from the interpreter sources at build time, so it can't drift from the code:

- `<!-- api: NAME -->` expands to the API reference for `lib/NAME.kiwi`, built from its `/# ... #/` docstrings (`@summary`, `@param type name : description`, `@return type : description`, `@throws`, `@example` between `---` lines). Every standard library page ends with one. To improve a package's reference, edit the docstrings in `lib/`.
- `<!-- opcodes -->` expands to the VM instruction set from the `<summary>` comments in `src/VM/Opcode.cs`.

## Style

- Write for someone who knows how to program but not Kiwi. Lead with what something does, then show it.
- Prefer short, runnable examples to long explanations.
- Describe what Kiwi actually does today. If behavior looks like a bug, document it plainly (a `:::caution` aside works well) and report it, rather than describing what it ought to do.
- Link to other pages with absolute paths that include the base: `/kiwi/language/functions/`.
