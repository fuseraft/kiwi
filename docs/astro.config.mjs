// @ts-check
import { defineConfig } from 'astro/config';
import starlight from '@astrojs/starlight';
import starlightLinksValidator from 'starlight-links-validator';
import { unified } from '@astrojs/markdown-remark';
import kiwiGrammar from './src/grammars/kiwi.tmLanguage.json' with { type: 'json' };
import remarkKiwiApi from './src/plugins/remark-kiwi-api.mjs';

export default defineConfig({
  site: 'https://fuseraft.ai',
  base: '/kiwi',
  trailingSlash: 'always',
  markdown: {
    processor: unified({ remarkPlugins: [remarkKiwiApi] }),
  },
  integrations: [
    starlight({
      title: 'Kiwi',
      description: 'A dynamically-typed scripting language with a concise, readable syntax and a batteries-included standard library.',
      logo: { src: './src/assets/logo.svg' },
      favicon: '/favicon.svg',
      social: [{ icon: 'github', label: 'GitHub', href: 'https://github.com/fuseraft/kiwi' }],
      editLink: { baseUrl: 'https://github.com/fuseraft/kiwi/edit/main/docs/' },
      lastUpdated: true,
      customCss: ['./src/styles/custom.css'],
      plugins: [starlightLinksValidator()],
      expressiveCode: {
        shiki: { langs: [/** @type {any} */ (kiwiGrammar)] },
        themes: ['github-dark-default', 'github-light-default'],
      },
      sidebar: [
        {
          label: 'Getting Started',
          items: ['getting-started/introduction', 'getting-started/installation', 'getting-started/tour'],
        },
        {
          label: 'Language',
          items: [
            'language/syntax',
            'language/variables',
            'language/types',
            'language/operators',
            'language/strings',
            'language/lists',
            'language/hashmaps',
            'language/control-flow',
            'language/loops',
            'language/functions',
            'language/lambdas',
            'language/scoping',
            'language/type-hints',
            'language/errors',
            'language/structs',
            'language/operator-overloading',
            'language/enums',
            'language/packages',
            'language/generators',
            'language/decorators',
            'language/events',
          ],
        },
        {
          label: 'Tools',
          items: ['tools/cli', 'tools/repl', 'tools/debugger', 'tools/testing'],
        },
        {
          label: 'Reference',
          items: ['reference/keywords', 'reference/builtin-methods', 'reference/global-functions'],
        },
        {
          label: 'Standard Library',
          collapsed: true,
          items: [{ autogenerate: { directory: 'stdlib' } }],
        },
        {
          label: 'Internals',
          collapsed: true,
          items: ['internals/vm'],
        },
      ],
    }),
  ],
});
