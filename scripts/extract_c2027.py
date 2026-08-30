import sys, re
seen = set()
for ln in sys.stdin:
    m = re.search(r"src\\([^\\(]+)\((\d+)[^)]*\).*'([^']+)'", ln)
    if m:
        seen.add((m.group(1), m.group(3)))
for f, t in sorted(seen):
    print(f"{f}\t{t}")
