#!/usr/bin/env bash
set -Eeuo pipefail

# ROOT-ONLY version. Keep this file in the project root.
ROOT="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"

if [[ ! -f "$ROOT/CMakeLists.txt" || ! -d "$ROOT/src" ]]; then
    echo "ERROR: this script must be kept in the project root." >&2
    exit 1
fi

echo "DANGER: this will permanently delete:"
echo "  src/"
echo "  srcorigjava/"
echo "  cmake/"
echo "  CMakeLists.txt"
echo "  CMakePresets.json"
echo "  AGENT.MD"
echo "  CODESTYLE.MD"
echo
echo "Project root:"
echo "  $ROOT"
echo
read -r -p "Type DELETE SOURCE TREE to continue: " answer

if [[ "$answer" != "DELETE SOURCE TREE" ]]; then
    echo "Cancelled."
    exit 1
fi

cd "$ROOT"

rm -rf -- src srcorigjava cmake
rm -f -- CMakeLists.txt CMakePresets.json AGENT.MD CODESTYLE.MD

echo "Deleted."
