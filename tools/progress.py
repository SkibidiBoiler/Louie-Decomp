"""How much of a game's ActionScript has a C++ translation, by functions and by lines.

usage: python -I tools/progress.py [<deob scripts dir>] [--by-class] [--missing <Class>]

A function counts as translated when some file under src/game carries `// AS: <Class>.<function>` for it
(the tag every translated function has). Lines are the function's own lines in the decompiled code, without
blank or brace-only lines. Functions whose C++ is still marked SKELETON count as translated: the figure is an
upper bound. Reads the owner's local decompiled code (outside the repo); prints to the terminal only.
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_DEOB = "C:/Users/alfre/Proyectos/datos-papas/extracted/pl2-deob/scripts"
# The game's own code: papaGame.* and the obfuscated packages that belong to it. The rest of the SWF
# (Flipline API, ads, Mochi, Playtomic, timeline scripts) is not ported (AGENTS.md).
GAME_DIRS = ["papaGame", "§_-9G§", "§_-4B§"]

HEAD = re.compile(r"\s*(override\s+)?(public|private|protected|internal)?\s*(static\s+)?"
                  r"function\s+(?:(get|set)\s+)?([^\s(]+)\s*\(")
TAG = re.compile(r"//\s*AS:\s*([^\s—(,;]+)")


def as_functions(path):
    """Yield (name, line count) for each function of one .as file."""
    lines = open(path, encoding="utf-8").read().split("\n")
    starts = [(i, m.group(5)) for i, l in enumerate(lines) if (m := HEAD.match(l))]
    for k, (i, name) in enumerate(starts):
        end = starts[k + 1][0] if k + 1 < len(starts) else len(lines)
        body = [l for l in lines[i:end] if l.strip() not in ("", "{", "}", "};")]
        yield name, len(body)


def cpp_tags():
    tags = set()
    for d, _, files in os.walk(os.path.join(ROOT, "src", "game")):
        for f in files:
            if f.endswith((".cpp", ".h")):
                for m in TAG.finditer(open(os.path.join(d, f), encoding="utf-8").read()):
                    parts = m.group(1).split(".")
                    if len(parts) >= 2:
                        tags.add((parts[-2], parts[-1]))
                    # `// AS: pkg.Class` heads a class file; it covers the constructor.
                    tags.add((parts[-1], parts[-1]))
    return tags


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    args = sys.argv[1:]
    deob = args.pop(0) if args and not args[0].startswith("--") else DEFAULT_DEOB
    by_class = "--by-class" in args
    missing = args[args.index("--missing") + 1] if "--missing" in args else None
    tags = cpp_tags()

    rows = []  # (class, done functions, functions, done lines, lines)
    for g in GAME_DIRS:
        for d, _, files in os.walk(os.path.join(deob, g)):
            for f in sorted(files):
                if not f.endswith(".as"):
                    continue
                cls = f[:-3]
                fn = list(as_functions(os.path.join(d, f)))
                if missing == cls:
                    for name, n in fn:
                        if (cls, name) not in tags:
                            print(f"  {name}  ({n} lines)")
                    return
                done = [(name, n) for name, n in fn if (cls, name) in tags]
                rows.append((cls, len(done), len(fn), sum(n for _, n in done), sum(n for _, n in fn)))

    if by_class:
        for cls, df, f, dl, l in sorted(rows, key=lambda r: r[3] / r[4] if r[4] else 1):
            print(f"{100 * dl / l if l else 100:5.1f} %  {df:3}/{f:<3} functions  {cls}")
    df = sum(r[1] for r in rows)
    f = sum(r[2] for r in rows)
    dl = sum(r[3] for r in rows)
    l = sum(r[4] for r in rows)
    started = sum(1 for r in rows if r[1])
    print(f"classes:   {started} of {len(rows)} started")
    print(f"functions: {df} of {f} translated ({100 * df / f:.1f} %)")
    print(f"lines:     {dl} of {l} ({100 * dl / l:.1f} %)")


if __name__ == "__main__":
    main()
