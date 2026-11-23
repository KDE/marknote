#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 Divyanshu <divyanshuchandra9027@gmail.com>
# SPDX-License-Identifier: LGPL-3.0-or-later
#
# Helper script to download, transpile, and update pele_bundle.js from upstream Pele.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PELE_VERSION="${1:-1.13.0}"

echo "Updating Pele bundle to version ${PELE_VERSION}..."

TMP_DIR="$(mktemp -d)"
trap 'rm -rf "${TMP_DIR}"' EXIT

cd "${TMP_DIR}"

npm pack "pele@${PELE_VERSION}"
tar -xzf pele-*.tgz

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

npx --yes esbuild package/dist/pele.global.js --target=es2017 >> "${TMP_DIR}/bundle.js"

cat << 'EOF' >> "${TMP_DIR}/bundle.js"

if (typeof pele !== "undefined") {
    globalThis.pele = pele;
}
EOF

mv "${TMP_DIR}/bundle.js" "${SCRIPT_DIR}/pele_bundle.js"
echo "Successfully updated ${SCRIPT_DIR}/pele_bundle.js to pele@${PELE_VERSION}."

