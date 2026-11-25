#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# Helper script to download, transpile, and update pele_bundle.js from upstream Pele.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PELE_VERSION="${1:-1.13.0}"
MATHJAX_VERSION="${2:-3.2.2}"

echo "Updating Pele bundle to pele@${PELE_VERSION} + mathjax-full@${MATHJAX_VERSION}..."

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

npm pack "pele@${PELE_VERSION}"
tar -xzf pele-*.tgz

echo "Installing mathjax-full@${MATHJAX_VERSION} for headless TeX-to-SVG math rendering..."
npm install "mathjax-full@${MATHJAX_VERSION}" --no-audit --no-fund

cat << 'EOF' > "${TMP_DIR}/bundle.js"
// SPDX-FileCopyrightText: 2024 Obsidian <https://obsidian.md>
// SPDX-License-Identifier: MIT

if (typeof globalThis === "undefined") {
    var globalThis = this;
}
if (typeof window === "undefined") {
    var window = this;
}
if (!String.prototype.trimStart) {
    String.prototype.trimStart = function () { return this.replace(/^\s+/, ""); };
}
if (!String.prototype.trimEnd) {
    String.prototype.trimEnd = function () { return this.replace(/\s+$/, ""); };
}
if (!String.prototype.trimLeft) {
    String.prototype.trimLeft = String.prototype.trimStart;
}
if (!String.prototype.trimRight) {
    String.prototype.trimRight = String.prototype.trimEnd;
}
if (!String.prototype.matchAll) {
    String.prototype.matchAll = function (regex) {
        var str = String(this);
        var flags = regex.flags !== undefined ? regex.flags : (regex.global ? "g" : "") + (regex.ignoreCase ? "i" : "") + (regex.multiline ? "m" : "") + (regex.unicode ? "u" : "");
        if (flags.indexOf("g") === -1) flags += "g";
        var rx = new RegExp(regex.source, flags);
        var matches = [];
        var match;
        while ((match = rx.exec(str)) !== null) {
            matches.push(match);
            if (!regex.global) break;
        }
        return matches;
    };
}
EOF

cat << 'EOF' > "${TMP_DIR}/math-resolver.js"
import { mathjax } from "mathjax-full/js/mathjax.js";
import { TeX } from "mathjax-full/js/input/tex.js";
import { SVG } from "mathjax-full/js/output/svg.js";
import { liteAdaptor } from "mathjax-full/js/adaptors/liteAdaptor.js";
import { RegisterHTMLHandler } from "mathjax-full/js/handlers/html.js";

import "mathjax-full/js/input/tex/base/BaseConfiguration.js";
import "mathjax-full/js/input/tex/ams/AmsConfiguration.js";
import "mathjax-full/js/input/tex/newcommand/NewcommandConfiguration.js";

let _adaptor = null;
let _tex = null;
let _svg = null;
let _html = null;

function getComponents() {
    if (!_adaptor) {
        _adaptor = liteAdaptor();
        RegisterHTMLHandler(_adaptor);
        _tex = new TeX({ packages: ["base", "ams", "newcommand"] });
        _svg = new SVG({ fontCache: "none" });
        _html = mathjax.document("", { InputJax: _tex, OutputJax: _svg });
    }
    return { adaptor: _adaptor, tex: _tex, svg: _svg, html: _html };
}

export function renderTexToSvg(texString, fontSize) {
    try {
        const { adaptor, tex, svg, html } = getComponents();
        const node = html.convert(texString, { display: false });
        const svgNode = adaptor.firstChild(node);
        if (!svgNode) return null;

        const widthAttr = adaptor.getAttribute(svgNode, "width") || "1ex";
        const heightAttr = adaptor.getAttribute(svgNode, "height") || "1ex";
        const widthEx = parseFloat(widthAttr) || 1;
        const heightEx = parseFloat(heightAttr) || 1;

        const exPx = (fontSize > 0 ? fontSize : 10) * 0.52;
        const width = widthEx * exPx;
        const height = heightEx * exPx;

        const viewBoxStr = adaptor.getAttribute(svgNode, "viewBox") || "";
        const parts = viewBoxStr.trim().split(/\s+/).map(Number);
        const minX = parts[0] || 0;
        const minY = parts[1] || 0;
        const vbW = parts[2] || 1;
        const vbH = parts[3] || 1;

        const scaleX = vbW > 0 ? width / vbW : 1;
        const scaleY = vbH > 0 ? height / vbH : 1;

        let innerContent = adaptor.innerHTML(svgNode);
        innerContent = innerContent.replace(/<path[^>]*\bd=""[^>]*>(?:<\/path>)?/g, "");
        const gContent = `<g transform="scale(${scaleX}, ${scaleY}) translate(${-minX}, ${-minY})">${innerContent}</g>`;

        return { width, height, svg: gContent };
    } catch (e) {
        return null;
    }
}

globalThis.renderTexToSvg = renderTexToSvg;
EOF

cat << 'EOF' > "${TMP_DIR}/mathjax-bundle.js"
// SPDX-FileCopyrightText: 2018-2022 The MathJax Consortium
// SPDX-License-Identifier: Apache-2.0

EOF
npx --yes esbuild "${TMP_DIR}/math-resolver.js" --bundle --minify --target=es2017 --define:PACKAGE_VERSION="\"${MATHJAX_VERSION}\"" >> "${TMP_DIR}/mathjax-bundle.js"
cat << 'EOF' >> "${TMP_DIR}/mathjax-bundle.js"

if (typeof renderTexToSvg !== "undefined") {
    globalThis.renderTexToSvg = renderTexToSvg;
}
EOF

npx --yes esbuild package/dist/pele.global.js --target=es2017 >> "${TMP_DIR}/bundle.js"

sed -i 's/\\p{L}\\p{N}/a-zA-Z0-9/g' "${TMP_DIR}/bundle.js"
sed -i 's/_ \.-/_ .\\-/g' "${TMP_DIR}/bundle.js"
sed -i 's/i\.math ? t\.math : void 0/i\.math ? (t\.math || globalThis\.renderTexToSvg) : void 0/g' "${TMP_DIR}/bundle.js"

cat << 'EOF' >> "${TMP_DIR}/bundle.js"

if (typeof pele !== "undefined") {
    globalThis.pele = pele;
}
EOF

mv "${TMP_DIR}/bundle.js" "${SCRIPT_DIR}/pele_bundle.js"
mv "${TMP_DIR}/mathjax-bundle.js" "${SCRIPT_DIR}/mathjax_bundle.js"
echo "Successfully updated ${SCRIPT_DIR}/pele_bundle.js (pele@${PELE_VERSION}) and mathjax_bundle.js (mathjax-full@${MATHJAX_VERSION})."

