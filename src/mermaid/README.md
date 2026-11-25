<!--
	SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
	SPDX-License-Identifier: CC-BY-SA-4.0
-->

# Mermaid

Marknote renders Mermaid diagrams offline using [Pele](https://github.com/obsidianmd/pele) and [MathJax](https://www.mathjax.org) (`mathjax-full`), executed inside a headless `QJSEngine`.

## Current Version

- **Pele**: 1.13.0 (MIT)
- **MathJax (`mathjax-full`)**: 3.2.2 (Apache-2.0)

## Updating

To update the vendored `pele_bundle.js`:

```sh
./src/mermaid/update-pele.sh [pele_version] [mathjax_version]
```

The script downloads both npm packages, bundles them with `esbuild`, and generates `pele_bundle.js`.
