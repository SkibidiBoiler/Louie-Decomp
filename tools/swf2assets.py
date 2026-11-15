#!/usr/bin/env python3
"""Convert a Flash SWF into the port's own asset format.

Reads the owner's SWF locally and writes, into --out (keep it outside the repo):
    symbols.bin            characters, shape bounds, class names and timelines (format: docs/asset-format.md)
    shapes/<N>x/<id>.png   every shape rasterised at scale N (by JPEXS FFDec)
    sounds/<id>.<ext>      every embedded sound (by JPEXS FFDec)
    manifest.json          human-readable summary

Usage:
    python -I tools/swf2assets.py --swf game.swf --out build/pl2 [--scales 1,2] [--skip-ffdec]
"""
import argparse
import json
import os
import shutil
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from swf_info import decompress, iter_tags, read_rect  # noqa: E402

DEFAULT_JAVA = r"C:\Program Files\Microsoft\jdk-21.0.13.21-hotspot\bin\java.exe"
DEFAULT_FFDEC = r"C:\Users\alfre\Proyectos\tools\ffdec\app\ffdec-cli.jar"

# Character kinds stored in symbols.bin.
KIND_SHAPE, KIND_SPRITE, KIND_MORPH, KIND_TEXT, KIND_EDITTEXT, KIND_BUTTON, KIND_BITMAP, KIND_SOUND, KIND_FONT = range(1, 10)
SHAPE_TAGS = {2, 22, 32, 83}
KIND_BY_TAG = {
    **{t: KIND_SHAPE for t in SHAPE_TAGS},
    39: KIND_SPRITE, 46: KIND_MORPH, 84: KIND_MORPH, 11: KIND_TEXT, 33: KIND_TEXT, 37: KIND_EDITTEXT,
    7: KIND_BUTTON, 34: KIND_BUTTON, 6: KIND_BITMAP, 20: KIND_BITMAP, 21: KIND_BITMAP, 35: KIND_BITMAP,
    36: KIND_BITMAP, 90: KIND_BITMAP, 14: KIND_SOUND, 10: KIND_FONT, 48: KIND_FONT, 75: KIND_FONT, 91: KIND_FONT,
}
NO_NAME = 0xFFFFFFFF


class Bits:
    """MSB-first bit reader used by SWF records (MATRIX, CXFORM)."""

    def __init__(self, data: bytes, pos: int):
        self.d, self.byte, self.bit = data, pos, 0

    def ub(self, n):
        v = 0
        for _ in range(n):
            v = (v << 1) | ((self.d[self.byte] >> (7 - self.bit)) & 1)
            self.bit += 1
            if self.bit == 8:
                self.bit, self.byte = 0, self.byte + 1
        return v

    def sb(self, n):
        v = self.ub(n)
        return v - (1 << n) if n and v & (1 << (n - 1)) else v

    def fb(self, n):
        return self.sb(n) / 65536.0

    def align(self):
        if self.bit:
            self.bit, self.byte = 0, self.byte + 1
        return self.byte


def read_matrix(data, pos):
    b = Bits(data, pos)
    a = d = 1.0
    bb = c = 0.0
    if b.ub(1):
        n = b.ub(5)
        a, d = b.fb(n), b.fb(n)
    if b.ub(1):
        n = b.ub(5)
        bb, c = b.fb(n), b.fb(n)
    n = b.ub(5)
    tx, ty = b.sb(n) / 20.0, b.sb(n) / 20.0
    return (a, bb, c, d, tx, ty), b.align()


def read_cxform(data, pos, with_alpha):
    b = Bits(data, pos)
    has_add, has_mult = b.ub(1), b.ub(1)
    n = b.ub(4)
    count = 4 if with_alpha else 3
    mult = [1.0, 1.0, 1.0, 1.0]
    add = [0.0, 0.0, 0.0, 0.0]
    if has_mult:
        for i in range(count):
            mult[i] = b.sb(n) / 256.0
    if has_add:
        for i in range(count):
            add[i] = b.sb(n) / 255.0
    return (tuple(mult), tuple(add)), b.align()


def skip_filters(data, pos):
    count = data[pos]
    pos += 1
    for _ in range(count):
        fid = data[pos]
        pos += 1
        if fid == 0:
            pos += 23
        elif fid == 1:
            pos += 9
        elif fid == 2:
            pos += 15
        elif fid == 3:
            pos += 27
        elif fid in (4, 7):
            n = data[pos]
            pos += 1 + 5 * n + 19
        elif fid == 5:
            mx, my = data[pos], data[pos + 1]
            pos += 2 + 8 + 4 * mx * my + 4 + 1
        elif fid == 6:
            pos += 80
        else:
            raise ValueError(f"unknown filter {fid}")
    return pos, count


def cstring(data, pos):
    end = data.index(0, pos)
    return data[pos:end].decode("utf-8", "replace"), end + 1


class Strings:
    def __init__(self):
        self.items, self.index = [], {}

    def add(self, s):
        if s not in self.index:
            self.index[s] = len(self.items)
            self.items.append(s)
        return self.index[s]


class Track:
    """One persistent child instance on a timeline: a character at a depth over a frame range."""

    def __init__(self, depth, char_id, frame, name=None, clip_depth=0, blend=0, filters=0):
        self.depth, self.char_id, self.start, self.end = depth, char_id, frame, None
        self.name, self.clip_depth, self.blend, self.filters = name, clip_depth, blend, filters
        self.keys = []  # (frame, matrix, cxform or None, ratio or None)
        self.matrix = (1.0, 0.0, 0.0, 1.0, 0.0, 0.0)
        self.cxform = None
        self.ratio = None


def parse_timeline(data, pos, end, frame_count):
    tracks, live, labels = [], {}, {}
    frame = 0
    for code, body in iter_tags(data, pos, end):
        if code == 1:  # ShowFrame
            frame += 1
        elif code == 43:  # FrameLabel
            labels[cstring(body, 0)[0]] = frame
        elif code in (5, 28):  # RemoveObject / RemoveObject2
            depth = struct.unpack_from("<H", body, 2 if code == 5 else 0)[0]
            t = live.pop(depth, None)
            if t:
                t.end = frame - 1
        elif code in (26, 70):  # PlaceObject2 / PlaceObject3
            f1 = body[0]
            f2 = body[1] if code == 70 else 0
            p = 2 if code == 70 else 1
            depth = struct.unpack_from("<H", body, p)[0]
            p += 2
            if code == 70 and (f2 & 0x08 or (f2 & 0x10 and f1 & 0x02)):
                _, p = cstring(body, p)
            char_id = None
            if f1 & 0x02:
                char_id = struct.unpack_from("<H", body, p)[0]
                p += 2
            matrix = cx = ratio = name = None
            clip = 0
            if f1 & 0x04:
                matrix, p = read_matrix(body, p)
            if f1 & 0x08:
                cx, p = read_cxform(body, p, True)
            if f1 & 0x10:
                ratio = struct.unpack_from("<H", body, p)[0]
                p += 2
            if f1 & 0x20:
                name, p = cstring(body, p)
            if f1 & 0x40:
                clip = struct.unpack_from("<H", body, p)[0]
                p += 2
            blend = nfilters = 0
            if code == 70:
                if f2 & 0x01:
                    p, nfilters = skip_filters(body, p)
                if f2 & 0x02:
                    blend = body[p]
                    p += 1

            t = live.get(depth)
            if char_id is not None and (t is None or not (f1 & 0x01) or t.char_id != char_id):
                if t:
                    t.end = frame - 1
                old = t
                t = Track(depth, char_id, frame, name, clip, blend, nfilters)
                if old and (f1 & 0x01):  # character swap keeps the old transform
                    t.matrix, t.cxform, t.ratio = old.matrix, old.cxform, old.ratio
                    t.name = t.name or old.name
                live[depth] = t
                tracks.append(t)
            elif t is None:
                continue
            if matrix is not None:
                t.matrix = matrix
            if cx is not None:
                t.cxform = cx
            if ratio is not None:
                t.ratio = ratio
            if name is not None:
                t.name = name
            if blend:
                t.blend = blend
            if t.keys and t.keys[-1][0] == frame:
                t.keys.pop()
            t.keys.append((frame, t.matrix, t.cxform, t.ratio))
    for t in tracks:
        if t.end is None:
            t.end = max(frame_count - 1, t.start)
        t.end = max(t.end, t.start)
    return tracks, labels


def parse_swf(path):
    buf = decompress(open(path, "rb").read())
    (xmin, xmax, ymin, ymax), pos = read_rect(buf, 8)
    frame_rate = struct.unpack_from("<H", buf, pos)[0] / 256.0
    root_frames = struct.unpack_from("<H", buf, pos + 2)[0]
    pos += 4
    chars, bounds, sprites, classes = {}, {}, {}, {}
    for code, body in iter_tags(buf, pos, len(buf)):
        kind = KIND_BY_TAG.get(code)
        if kind:
            cid = struct.unpack_from("<H", body, 0)[0]
            chars[cid] = kind
            if kind == KIND_SHAPE:
                (x0, x1, y0, y1), _ = read_rect(body, 2)
                bounds[cid] = (x0 / 20.0, y0 / 20.0, (x1 - x0) / 20.0, (y1 - y0) / 20.0)
            elif kind == KIND_SPRITE:
                fc = struct.unpack_from("<H", body, 2)[0]
                sprites[cid] = (fc,) + parse_timeline(body, 4, len(body), fc)
        elif code == 76:  # SymbolClass
            n = struct.unpack_from("<H", body, 0)[0]
            p = 2
            for _ in range(n):
                cid = struct.unpack_from("<H", body, p)[0]
                name, p = cstring(body, p + 2)
                classes[name] = cid
    sprites[0] = (root_frames,) + parse_timeline(buf, pos, len(buf), root_frames)
    stage = {"width": (xmax - xmin) / 20.0, "height": (ymax - ymin) / 20.0, "fps": frame_rate}
    return stage, chars, bounds, sprites, classes


def write_symbols(path, chars, bounds, sprites, classes):
    strings = Strings()
    out = bytearray()
    chars_blob = bytearray(struct.pack("<I", len(chars)))
    for cid in sorted(chars):
        chars_blob += struct.pack("<HB", cid, chars[cid])
        if chars[cid] == KIND_SHAPE:
            chars_blob += struct.pack("<4f", *bounds[cid])
    classes_blob = bytearray(struct.pack("<I", len(classes)))
    for name, cid in sorted(classes.items()):
        classes_blob += struct.pack("<IH", strings.add(name), cid)
    sprites_blob = bytearray(struct.pack("<I", len(sprites)))
    for sid in sorted(sprites):
        fc, tracks, labels = sprites[sid]
        sprites_blob += struct.pack("<HHH", sid, fc, len(labels))
        for label, frame in sorted(labels.items(), key=lambda kv: kv[1]):
            sprites_blob += struct.pack("<IH", strings.add(label), frame)
        sprites_blob += struct.pack("<I", len(tracks))
        for t in tracks:
            name = strings.add(t.name) if t.name else NO_NAME
            sprites_blob += struct.pack("<HHIHBBHHI", t.depth, t.char_id, name, t.clip_depth, t.blend,
                                        min(t.filters, 255), t.start, t.end, len(t.keys))
            for frame, m, cx, ratio in t.keys:
                flags = (1 if cx else 0) | (2 if ratio is not None else 0)
                sprites_blob += struct.pack("<HB6f", frame, flags, *m)
                if cx:
                    sprites_blob += struct.pack("<8f", *cx[0], *cx[1])
                if ratio is not None:
                    sprites_blob += struct.pack("<H", ratio)
    out += b"PLSY" + struct.pack("<I", 1)
    out += struct.pack("<I", len(strings.items))
    for s in strings.items:
        raw = s.encode("utf-8")
        out += struct.pack("<H", len(raw)) + raw
    out += chars_blob + classes_blob + sprites_blob
    Path(path).write_bytes(out)
    return len(out)


def run_ffdec(java, ffdec, args, log):
    cmd = [java, "-Xmx4g", "-jar", ffdec] + args
    with open(log, "a", encoding="utf-8") as fh:
        res = subprocess.run(cmd, stdout=fh, stderr=subprocess.STDOUT)
    if res.returncode != 0:
        raise RuntimeError(f"FFDec failed ({res.returncode}): see {log}")


def export_media(swf, out, scales, java, ffdec):
    log = out / "ffdec.log"
    log.write_text("", encoding="utf-8")
    for s in scales:
        dest = out / "shapes" / f"{s}x"
        if dest.exists():
            shutil.rmtree(dest)
        with tempfile.TemporaryDirectory(dir=out) as tmp:
            run_ffdec(java, ffdec, ["-format", "shape:png", "-zoom", str(s), "-export", "shape", tmp, str(swf)], log)
            src = Path(tmp) / "shapes" if (Path(tmp) / "shapes").is_dir() else Path(tmp)
            shutil.move(str(src), str(dest))
        print(f"  shapes {s}x: {len(list(dest.glob('*.png')))} png")
    dest = out / "sounds"
    if dest.exists():
        shutil.rmtree(dest)
    with tempfile.TemporaryDirectory(dir=out) as tmp:
        run_ffdec(java, ffdec, ["-export", "sound", tmp, str(swf)], log)
        src = Path(tmp) / "sounds" if (Path(tmp) / "sounds").is_dir() else Path(tmp)
        shutil.move(str(src), str(dest))
    print(f"  sounds: {len(list(dest.iterdir()))} files")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--swf", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--scales", default="1,2")
    ap.add_argument("--skip-ffdec", action="store_true", help="only rewrite symbols.bin and manifest.json")
    ap.add_argument("--java", default=DEFAULT_JAVA)
    ap.add_argument("--ffdec", default=DEFAULT_FFDEC)
    args = ap.parse_args()

    swf, out = Path(args.swf).resolve(), Path(args.out).resolve()
    repo = Path(__file__).resolve().parent.parent
    if repo in out.parents or out == repo:
        sys.exit(f"refusing to write game assets inside the repository: {out}")
    out.mkdir(parents=True, exist_ok=True)
    scales = [int(s) for s in args.scales.split(",")]

    stage, chars, bounds, sprites, classes = parse_swf(swf)
    size = write_symbols(out / "symbols.bin", chars, bounds, sprites, classes)
    tracks = sum(len(v[1]) for v in sprites.values())
    keys = sum(len(t.keys) for v in sprites.values() for t in v[1])
    print(f"symbols.bin: {size:,} bytes; {len(chars)} characters, {len(sprites)} timelines, "
          f"{tracks} tracks, {keys} keys, {len(classes)} classes")
    if not args.skip_ffdec:
        export_media(swf, out, scales, args.java, args.ffdec)

    kinds = {}
    for k in chars.values():
        kinds[k] = kinds.get(k, 0) + 1
    manifest = {
        "format": 1,
        "source": swf.name,
        "stage": stage,
        "scales": scales,
        "characters": {name: kinds.get(k, 0) for name, k in [
            ("shape", KIND_SHAPE), ("sprite", KIND_SPRITE), ("morph", KIND_MORPH), ("text", KIND_TEXT),
            ("edittext", KIND_EDITTEXT), ("button", KIND_BUTTON), ("bitmap", KIND_BITMAP),
            ("sound", KIND_SOUND), ("font", KIND_FONT)]},
        "timelines": len(sprites),
        "classes": len(classes),
    }
    (out / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    return 0


if __name__ == "__main__":
    sys.exit(main())
