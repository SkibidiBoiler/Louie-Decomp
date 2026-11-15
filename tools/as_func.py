"""Print one function of the decompiled ActionScript, compactly (no brace-only or blank lines).

usage: python -I tools/as_func.py <deob scripts dir> <Class> <method> [<method> ...]
       python -I tools/as_func.py <deob scripts dir> <Class> --list
Reads the owner's local decompiled code (outside the repo); prints to the terminal only.
"""
import os
import re
import sys


def find_class(root, name):
    for d, _, files in os.walk(root):
        if name + ".as" in files:
            return os.path.join(d, name + ".as")
    raise SystemExit("class not found: " + name)


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    root, cls = sys.argv[1], sys.argv[2]
    lines = open(find_class(root, cls), encoding="utf-8").read().split("\n")
    head = re.compile(r"\s*(override\s+)?(public|private|protected|internal)?\s*(static\s+)?function\s+(\S+?)\(")
    if sys.argv[3] == "--list":
        for i, l in enumerate(lines):
            m = head.match(l)
            if m:
                print(i + 1, m.group(4))
        return
    for want in sys.argv[3:]:
        for i, l in enumerate(lines):
            m = head.match(l)
            if not m or m.group(4) != want:
                continue
            depth, started = 0, False
            for j in range(i, len(lines)):
                t = lines[j]
                depth += t.count("{") - t.count("}")
                started = started or "{" in t
                s = t.strip()
                if s and s not in ("{", "}"):
                    print("%d: %s" % (j + 1, t.rstrip()[6:] if t.startswith("      ") else t.rstrip()))
                if started and depth <= 0:
                    break
            print()


main()
