#!/usr/bin/env bash
set -Eeuo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/_build_common.sh"
ensure_zip_tool
cd "$ROOT"
rm -f mcrelease.zip
items=(src srcorigjava cmake CMakeLists.txt CMakePresets.json AGENT.MD CODESTYLE.MD)
existing=()
for item in "${items[@]}"; do [[ -e "$item" ]] && existing+=("$item"); done
((${#existing[@]})) || fail "No release source files were found."
zip -r mcrelease.zip "${existing[@]}"
echo "Created: $ROOT/mcrelease.zip"
