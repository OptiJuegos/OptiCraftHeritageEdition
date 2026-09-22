#!/usr/bin/env bash
set -Eeuo pipefail
source "$(dirname "${BASH_SOURCE[0]}")/_build_common.sh"
ensure_base_tools
MODE="${1:-}"
python3 - "$ROOT" "$MODE" <<'PY'
import os, sys, time
root, mode = sys.argv[1:3]
now = time.time()
limit = now + 2
all_files = mode.lower() == "all"
fixed = failed = build_count = 0
for base, dirs, files in os.walk(root):
    dirs[:] = [d for d in dirs if d != ".git"]
    for name in files:
        path = os.path.join(base, name)
        try:
            st = os.stat(path)
            if not all_files and st.st_mtime <= limit:
                continue
            os.utime(path, (now, now))
            fixed += 1
            rel = os.path.relpath(path, root)
            if rel == "build" or rel.startswith("build" + os.sep):
                build_count += 1
        except OSError:
            failed += 1
if fixed == 0:
    print("Timestamps are sane; nothing to restamp.")
else:
    print(f"Restamped {fixed} file(s).")
if failed:
    print(f"WARNING: {failed} file(s) could not be restamped.")
if build_count:
    print(f"WARNING: {build_count} changed file(s) are under build/; a clean reconfigure is recommended.")
PY
