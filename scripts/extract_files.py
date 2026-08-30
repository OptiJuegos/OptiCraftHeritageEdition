"""Extracts unique source file basenames from a build log of MSVC errors."""
import sys, re

files = set()
for ln in sys.stdin:
    m = re.search(r"src\\([^\\(]+)\(", ln)
    if m:
        files.add(m.group(1))
for f in sorted(files):
    print(f)
