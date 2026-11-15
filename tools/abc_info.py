#!/usr/bin/env python3
"""Measure the ActionScript 3 (ABC) code inside a SWF: classes, packages and bytecode size.

Read-only and local. Usage:
    python -I tools/abc_info.py game.swf                 # per-package summary
    python -I tools/abc_info.py game.swf --classes       # also list every class with its code size
    python -I tools/abc_info.py a.swf --compare b.swf    # classes shared between two games
"""
import struct
import sys
from collections import defaultdict
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from swf_info import decompress, iter_tags, read_rect  # noqa: E402


class Reader:
    def __init__(self, data: bytes):
        self.d = data
        self.p = 0

    def u8(self):
        v = self.d[self.p]
        self.p += 1
        return v

    def u16(self):
        v = struct.unpack_from("<H", self.d, self.p)[0]
        self.p += 2
        return v

    def u30(self):
        result = shift = 0
        for _ in range(5):
            b = self.u8()
            result |= (b & 0x7F) << shift
            shift += 7
            if not b & 0x80:
                break
        return result

    def skip(self, n):
        self.p += n


class Abc:
    def __init__(self, data: bytes):
        r = Reader(data)
        r.u16()
        r.u16()
        n = r.u30()
        for _ in range(max(n - 1, 0)):
            r.u30()
        n = r.u30()
        for _ in range(max(n - 1, 0)):
            r.u30()
        n = r.u30()
        r.skip(8 * max(n - 1, 0))
        self.strings = [""]
        for _ in range(max(r.u30() - 1, 0)):
            ln = r.u30()
            self.strings.append(r.d[r.p:r.p + ln].decode("utf-8", "replace"))
            r.skip(ln)
        self.namespaces = [(0, 0)]
        for _ in range(max(r.u30() - 1, 0)):
            self.namespaces.append((r.u8(), r.u30()))
        for _ in range(max(r.u30() - 1, 0)):
            for _ in range(r.u30()):
                r.u30()
        self.multinames = [(None, 0)]
        for _ in range(max(r.u30() - 1, 0)):
            kind = r.u8()
            if kind in (0x07, 0x0D):
                ns, name = r.u30(), r.u30()
                self.multinames.append((ns, name))
            elif kind in (0x0F, 0x10):
                self.multinames.append((None, r.u30()))
            elif kind in (0x11, 0x12):
                self.multinames.append((None, 0))
            elif kind in (0x09, 0x0E):
                name = r.u30()
                r.u30()
                self.multinames.append((None, name))
            elif kind in (0x1B, 0x1C):
                r.u30()
                self.multinames.append((None, 0))
            elif kind == 0x1D:
                q = r.u30()
                for _ in range(r.u30()):
                    r.u30()
                self.multinames.append(self.multinames[q] if q < len(self.multinames) else (None, 0))
            else:
                raise ValueError(f"unknown multiname kind 0x{kind:x}")

        method_count = r.u30()
        for _ in range(method_count):
            pc = r.u30()
            r.u30()
            for _ in range(pc):
                r.u30()
            r.u30()
            flags = r.u8()
            if flags & 0x08:
                for _ in range(r.u30()):
                    r.u30()
                    r.u8()
            if flags & 0x80:
                for _ in range(pc):
                    r.u30()
        for _ in range(r.u30()):
            r.u30()
            for _ in range(r.u30()):
                r.u30()
                r.u30()

        class_count = r.u30()
        self.classes = []  # (package, name, [method indices])
        for _ in range(class_count):
            name = r.u30()
            r.u30()
            flags = r.u8()
            if flags & 0x08:
                r.u30()
            for _ in range(r.u30()):
                r.u30()
            iinit = r.u30()
            methods = [iinit] + self._traits(r)
            pkg, cname = self.qname(name)
            self.classes.append([pkg, cname, methods])
        for i in range(class_count):
            cinit = r.u30()
            self.classes[i][2] += [cinit] + self._traits(r)
        for _ in range(r.u30()):
            r.u30()
            self._traits(r)

        self.body_size = {}
        for _ in range(r.u30()):
            m = r.u30()
            r.u30()
            r.u30()
            r.u30()
            r.u30()
            ln = r.u30()
            r.skip(ln)
            self.body_size[m] = ln
            for _ in range(r.u30()):
                for _ in range(5):
                    r.u30()
            self._traits(r)

    def qname(self, mn):
        ns, name = self.multinames[mn]
        pkg = self.strings[self.namespaces[ns][1]] if ns else ""
        return pkg, self.strings[name]

    def _traits(self, r):
        methods = []
        for _ in range(r.u30()):
            r.u30()
            kind = r.u8()
            k = kind & 0x0F
            if k in (0, 6):
                r.u30()
                r.u30()
                if r.u30():
                    r.u8()
            elif k in (1, 2, 3):
                r.u30()
                methods.append(r.u30())
            elif k in (4, 5):
                r.u30()
                r.u30()
            if kind & 0x40:
                for _ in range(r.u30()):
                    r.u30()
        return methods


def load(path):
    buf = decompress(open(path, "rb").read())
    _, pos = read_rect(buf, 8)
    classes = {}
    total = 0
    for code, body in iter_tags(buf, pos + 4, len(buf)):
        if code in (72, 82):
            data = body if code == 72 else body[body.index(0, 4) + 1:]
            abc = Abc(data)
            total += sum(abc.body_size.values())
            for pkg, name, methods in abc.classes:
                size = sum(abc.body_size.get(m, 0) for m in set(methods))
                key = f"{pkg}.{name}" if pkg else name
                classes[key] = (pkg, size, len(set(methods)))
    return classes, total


AD_HINTS = ("flipline", "adsystem", "failsafe", "mochi", "cpmstar", "branding", "fla.", "_fla")


def main(argv):
    path = argv[1]
    classes, total = load(path)
    attributed = sum(s for _, s, _ in classes.values())
    print(f"{path}")
    print(f"  {len(classes)} classes, {total:,} bytes of AVM2 bytecode "
          f"({attributed:,} in class methods, {total - attributed:,} in closures/scripts)")
    by_pkg = defaultdict(lambda: [0, 0])
    for _, (pkg, size, _) in classes.items():
        by_pkg[pkg or "<top level>"][0] += 1
        by_pkg[pkg or "<top level>"][1] += size
    print(f"  {'package':45} {'classes':>8} {'bytecode':>10}")
    for pkg, (n, size) in sorted(by_pkg.items(), key=lambda kv: -kv[1][1])[:25]:
        print(f"  {pkg[:45]:45} {n:8} {size:10,}")

    timeline = {k: v for k, v in classes.items() if "_fla" in k or "fla." in k}
    ads = {k: v for k, v in classes.items() if any(h in k.lower() for h in AD_HINTS) and k not in timeline}
    print(f"  timeline classes (*_fla.*): {len(timeline)}, {sum(v[1] for v in timeline.values()):,} bytes")
    print(f"  ad/API/branding classes:    {len(ads)}, {sum(v[1] for v in ads.values()):,} bytes")

    if "--classes" in argv:
        for k, (pkg, size, nm) in sorted(classes.items(), key=lambda kv: -kv[1][1]):
            print(f"    {size:8,}  {nm:4} methods  {k}")

    if "--compare" in argv:
        other, _ = load(argv[argv.index("--compare") + 1])
        shared = classes.keys() & other.keys()
        same = [k for k in shared if classes[k][1] == other[k][1]]
        print(f"  compare: {len(shared)} class names shared, {len(same)} with identical code size "
              f"({sum(classes[k][1] for k in same):,} bytes); "
              f"{len(classes.keys() - other.keys())} only here, {len(other.keys() - classes.keys())} only there")
    return 0


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv))
