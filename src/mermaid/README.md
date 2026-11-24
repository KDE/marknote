<!--
	SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
	SPDX-License-Identifier: CC-BY-SA-4.0
-->

# Mermaid

Marknote renders Mermaid diagrams offline using [Pele](https://github.com/obsidianmd/pele), a lightweight engine executed inside a headless `QJSEngine`.

## Current Version

- **Pele**: 1.13.0
- **License**: MIT

## Updating

To update the vendored `pele_bundle.js` to a newer version:

```sh
./src/mermaid/update-pele.sh <version>
```

The script downloads the npm package, bundles it with esbuild, and generates `pele_bundle.js`.
