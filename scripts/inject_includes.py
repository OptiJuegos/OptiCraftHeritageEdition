"""Inject missing #include directives into .cpp files based on C2027 errors.

Reads scripts/c2027.tsv (file<TAB>type per line), then for each unique pair
adds  #include "Type.h"  to the .cpp file if not already present.
Java helper types (Random) live under java/Random.h.
"""
import os
import sys
import re

SRC_ROOT = os.path.join(os.path.dirname(__file__), '..', 'src', 'net', 'minecraft', 'src')
SRC_ROOT = os.path.abspath(SRC_ROOT)
TSV = os.path.join(os.path.dirname(__file__), 'c2027.tsv')

# Types that live outside the flat net/minecraft/src layout
SPECIAL = {
    'Random': '"java/Random.h"',
}

def header_for(type_name: str) -> str:
    if type_name in SPECIAL:
        return SPECIAL[type_name]
    return f'"{type_name}.h"'

def main() -> int:
    pairs: dict[str, set[str]] = {}
    with open(TSV, encoding='utf-8') as fh:
        for ln in fh:
            ln = ln.strip()
            if not ln:
                continue
            f, t = ln.split('\t')
            pairs.setdefault(f, set()).add(t)

    changed = 0
    for fname, types in sorted(pairs.items()):
        path = os.path.join(SRC_ROOT, fname)
        if not os.path.isfile(path):
            print(f'SKIP missing: {fname}', file=sys.stderr)
            continue
        with open(path, encoding='utf-8') as fh:
            src = fh.read()
        original = src
        wanted = sorted(header_for(t) for t in types)
        injection = '\n'.join(f'#include {w}' for w in wanted if f'#include {w}' not in src)
        if not injection:
            continue
        # Insert after the last existing #include line, or after pragma once.
        m = list(re.finditer(r'^#include[^\n]*\n', src, flags=re.MULTILINE))
        if m:
            insert_at = m[-1].end()
            src = src[:insert_at] + injection + '\n' + src[insert_at:]
        else:
            # No includes: put after pragma once or at top
            m2 = re.search(r'^#pragma once[^\n]*\n', src, flags=re.MULTILINE)
            if m2:
                insert_at = m2.end()
                src = src[:insert_at] + '\n' + injection + '\n' + src[insert_at:]
            else:
                src = injection + '\n' + src
        if src != original:
            try:
                with open(path, 'w', encoding='utf-8') as fh:
                    fh.write(src)
                changed += 1
                print(f'PATCH {fname}: + {len(injection.splitlines())} include(s)')
            except PermissionError as e:
                print(f'LOCKED (skipped) {fname}: {e}', file=sys.stderr)
    print(f'done: patched {changed} files')
    return 0

if __name__ == '__main__':
    sys.exit(main())
