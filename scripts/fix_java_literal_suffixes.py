"""Rewrites Java-style numeric literal suffixes to C++ equivalents.

  0.5D / 0D       -> 0.5  / 0.0
  0.5F / 0F       -> 0.5f / 0.0f
  64L             -> 64LL

Only literal suffixes are touched — identifiers like `myVarD` stay intact, and
the regex requires a digit before the suffix and a non-identifier char after.
"""
import os
import re
import sys

ROOT = os.path.join(os.path.dirname(__file__), '..', 'src', 'net', 'minecraft', 'src')
ROOT = os.path.abspath(ROOT)

# Pattern: optional leading dot+digits or digits+dot+digits, then suffix D|F|L, then non-ident.
LIT = re.compile(r'(?<![A-Za-z_0-9])(\d+\.\d*|\.\d+|\d+)([DFL])(?![A-Za-z_0-9])')

def rewrite(text: str) -> str:
    def sub(m: re.Match) -> str:
        num, suf = m.group(1), m.group(2)
        if suf == 'D':
            return num if '.' in num else num + '.0'
        if suf == 'F':
            return (num if '.' in num else num + '.0') + 'f'
        if suf == 'L':
            return num + 'LL'
        return m.group(0)
    return LIT.sub(sub, text)

def main():
    targets = sys.argv[1:]
    if not targets:
        # Sweep every .cpp / .h.
        for root, _, files in os.walk(ROOT):
            for f in files:
                if f.endswith(('.cpp', '.h')):
                    targets.append(os.path.join(root, f))
    else:
        targets = [os.path.join(ROOT, t) for t in targets]

    changed = 0
    for path in targets:
        try:
            with open(path, 'r', encoding='utf-8') as fh:
                src = fh.read()
        except (FileNotFoundError, UnicodeDecodeError):
            continue
        new = rewrite(src)
        if new != src:
            try:
                with open(path, 'w', encoding='utf-8') as fh:
                    fh.write(new)
                changed += 1
            except PermissionError as e:
                print(f'LOCKED {path}: {e}', file=sys.stderr)
    print(f'fixed {changed} files')

if __name__ == '__main__':
    main()
