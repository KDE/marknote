<!--
SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
SPDX-License-Identifier: CC-BY-SA-4.0
-->

# Mermaid Rendering in Marknote

Marknote renders Mermaid diagrams natively using Obsidian's [Pele](https://github.com/obsidianmd/pele) engine executed in a headless `QJSEngine` on a background thread.

Unlike standard `mermaid.js` (>3 MB, requires browser DOM), Pele is zero-dependency, lightweight, and compiles down to ~220 KB via Qt RCC zlib compression.

---

## Architecture & Integration

1. **Execution**: Runs `pele_bundle.js` inside an isolated `QJSEngine` worker thread without browser DOM dependencies.
2. **Font Metrics**: Uses C++ `QFontMetricsF` (`NativeMeasurer`) to measure text dimensions directly during layout using the note's active font.
3. **Theming & SVG Post-Processing**: `MermaidCssFlattener` injects Kirigami color scheme values and flattens nested `<tspan>` tags into standalone `<text>` elements to maintain compatibility with Qt's SVG Tiny 1.2 renderer.
4. **Runtime Polyfills**: `JsEngineMermaidRenderer` injects basic prototype polyfills (`Array.prototype.flat`, `String.prototype.replaceAll`, `TypedArray.prototype.sort`) required by Pele's parsers.

---

## Supported Diagram Types

Diagrams are automatically detected via ````mermaid` or ````mmd` code blocks, or by their starting keyword. Over 35 diagram types are supported, including `flowchart`, `sequenceDiagram`, `classDiagram`, `stateDiagram`, `erDiagram`, `gantt`, `timeline`, `gitGraph`, `pie`, `mindmap`, `C4Component`, `sankey-beta`, etc.

---

## Updating the Bundle

- **Upstream**: [obsidianmd/pele](https://github.com/obsidianmd/pele) (MIT License)
- **Vendored Version**: `1.13.0` (`package/dist/pele.global.js`)

To update or regenerate the committed bundle:
```bash
./src/mermaid/update-pele.sh <version>
```

The bundle is committed to the repository to allow hermetic, offline builds across Linux distributions, FreeBSD, and Android without requiring a Node.js/npm toolchain at compile time.
