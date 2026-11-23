<!--
SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
SPDX-License-Identifier: CC-BY-SA-4.0
-->

# Native Mermaid Diagram Rendering in Marknote

This directory implements native Mermaid diagram rendering in Marknote using Obsidian's MIT-licensed **Pele** engine executing in an isolated background `QJSEngine`.

---

## 1. Pele Bundle Provenance & Specifications

- **Upstream Project**: [Pele](https://github.com/obsidianmd/pele) (`obsidianmd/pele`)
- **Author & Copyright**: Steph Ango (`kepano`), Obsidian (`https://obsidian.md`)
- **License**: MIT (see `LICENSES/MIT.txt` and embedded SPDX headers)
- **Vendored Version**: `1.13.0` (published on npm as `pele@1.13.0`, release 1.13.0)
- **Source File**: `package/dist/pele.global.js` (official IIFE build from upstream package)

### Why Pele Instead of mermaid.js?
1. **Lightweight & Fast**: Pele is a zero-dependency, DOM-free rendering engine. Diagrams parse and layout in milliseconds, compared to standard `mermaid.js` which exceeds 3 MB and requires full browser DOM / JSDOM.
2. **Pluggable Font Measurer**: Allows C++ `QFontMetricsF` to measure text dimensions directly during layout.
3. **Dynamic Theming**: Outputs clean SVG styled via CSS variables (`--pele-*`), allowing seamless integration with Kirigami / `KColorScheme` themes and font settings.

---

## 2. Bundle Construction & Transpilation

Qt's ECMAScript engine (`QJSEngine`, powered by Qt Quick's QV4) runs without a browser DOM and has target constraints on certain modern JavaScript syntax features across embedded and desktop platforms.

`pele_bundle.js` is generated from `dist/pele.global.js` via the following pipeline:

1. **Polyfill Header**:
   - Polyfills `globalThis` and `window` for headless `QJSEngine`.
   - Polyfills `String.prototype.trimStart`, `trimEnd`, and `matchAll`.
2. **ES2017 Downleveling**:
   - Transpiled via `esbuild --target=es2017` to lower ES2020+ constructs (such as optional chaining `?.` and nullish coalescing `??`) to ES2017 syntax natively compatible with Qt 6 `QJSEngine` across all platforms (Linux, Android, Windows, FreeBSD).
3. **Global Registration**:
   - Assigns `globalThis.pele = pele;` so C++ can directly invoke `pele.render(...)`.

### Bundle Size & Footprint
- **Uncompressed JavaScript**: ~1,079 KB (1.05 MB)
- **Compressed in Binary**: Embedded via Qt's resource compiler (`pele.qrc`). Qt RCC applies zlib compression automatically, compiling down to approximately **220 KB** within `libmarknote_static.a`.

---

## 3. Regenerating or Updating the Bundle

A regeneration script is provided at `src/mermaid/update-pele.sh`.

To regenerate or upgrade to a newer upstream release of Pele:
```bash
chmod +x src/mermaid/update-pele.sh
./src/mermaid/update-pele.sh 1.13.0
```

### Why is the bundle committed rather than generated at build time?
1. **Offline & Hermetic Packaging**: KDE software is built across Linux distributions (Debian, Fedora, openSUSE, Arch), FreeBSD ports, and Flatpak in strictly offline environments where external network access (such as `npm install`) is disabled.
2. **Toolchain Minimization**: Marknote is a lightweight C++/Qt application. Requiring Node.js and an npm ecosystem toolchain at build time would complicate builds on Android and FreeBSD (supported in `.kde-ci.yml`) and add unnecessary build dependencies.
3. **Determinism**: Committing the pre-transpiled resource ensures reproducible builds across all target platforms.
