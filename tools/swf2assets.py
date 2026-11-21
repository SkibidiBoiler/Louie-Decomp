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
import math
import os
import re
import shutil
import struct
import subprocess
import sys
import tempfile
import zlib
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


def _morph_fill0_fields(body, p):
    """Bit offsets of the style-change flags in a morph shape's start records whose fill sits only on side 0.

    Returns None when the shape sets fill style 1 anywhere (then it is left alone)."""
    b = Bits(body, p)
    nf, nl = b.ub(4), b.ub(4)
    found = []
    while True:
        if not b.ub(1):
            at = b.byte * 8 + b.bit
            flags = b.ub(5)
            if flags == 0:
                return found
            if flags & 0x10:  # new styles: not in morph shapes
                return None
            if flags & 1:
                n = b.ub(5)
                b.sb(n)
                b.sb(n)
            if flags & 4:  # any fill style 1: leave the shape alone
                return None
            if flags & 2:
                b.ub(nf)
                found.append(at)
            if flags & 8:
                b.ub(nl)
        elif b.ub(1):  # straight edge
            n = b.ub(4) + 2
            if b.ub(1):
                b.sb(n)
                b.sb(n)
            else:
                b.ub(1)
                b.sb(n)
        else:  # curved edge
            n = b.ub(4) + 2
            for _ in range(4):
                b.sb(n)


def fix_morph_fill_sides(buf):
    """A copy of the (uncompressed) SWF for FFDec where every DefineMorphShape filled only on side 0 of its edges
    is filled on side 1 instead (every fill style 0 change becomes a fill style 1 change).

    Flash fills a closed outline the same whichever side of the edges carries the fill, but FFDec 26 renders a
    morph shape filled on side 0 inverted (everything in its bounds but the outline). The blimps' balloon is masked
    by such a circle, so most frames of every `object_blimp_*` sheet came out without the balloon."""
    out = bytearray(buf)
    _, pos = read_rect(buf, 8)
    pos += 4
    fixed = 0
    while pos + 2 <= len(buf):
        code_len = struct.unpack_from("<H", buf, pos)[0]
        pos += 2
        code, length = code_len >> 6, code_len & 0x3F
        if length == 0x3F:
            length = struct.unpack_from("<I", buf, pos)[0]
            pos += 4
        if code == 46:  # DefineMorphShape (fills and lines in the start records only)
            body = buf[pos:pos + length]
            p = 2
            _, p = read_rect(body, p)
            _, p = read_rect(body, p)
            p += 4  # offset of the end edges
            n = body[p]
            p += 1
            if n == 0xFF:
                n = struct.unpack_from("<H", body, p)[0]
                p += 2
            for _ in range(n):
                kind = body[p]
                p += 1
                if kind != 0:  # gradients and bitmaps: not needed, and their size varies
                    break
                p += 8
            else:
                nlines = body[p]
                p += 1
                if nlines == 0xFF:
                    nlines = struct.unpack_from("<H", body, p)[0]
                    p += 2
                p += nlines * 12
                fields = _morph_fill0_fields(body, p)
                for at in fields or []:
                    # Flags are 5 bits MSB first: NewStyles, LineStyle, FillStyle1, FillStyle0, MoveTo. With only
                    # FillStyle0 set, its index field is where a lone FillStyle1 index would be.
                    for bit, value in ((at + 2, 1), (at + 3, 0)):
                        byte = pos + bit // 8
                        mask = 0x80 >> (bit % 8)
                        out[byte] = (out[byte] | mask) if value else (out[byte] & ~mask)
                if fields:
                    fixed += 1
        pos += length
        if code == 0:
            break
    out[0:3] = b"FWS"
    struct.pack_into("<I", out, 4, len(out))
    return bytes(out), fixed


def export_sheets(swf, out, scales, java, ffdec, names, chars, classes):
    """Bake the classes the game rasterises at runtime: sprites (frame 1) at every scale, bitmaps as-is.

    Writes sheets/<N>x/<name>.png (vector) or sheets/1x/<name>.png|jpg (bitmap) and sheets/index.txt with one
    line per sheet: "<name> <origin x> <origin y> vector|bitmap <width> <height>" (origin = top-left of the PNG in
    symbol units; width/height = the BitmapData size the original allocates, ceil of the clip's width/height,
    covering symbol coordinates 0..width x 0..height; 0 0 for bitmaps = the image size).
    """
    log = out / "ffdec.log"
    root = out / "sheets"
    if root.exists():
        shutil.rmtree(root)
    root.mkdir(parents=True)
    by_id = {}
    for n in names:
        cid = classes.get(n)
        if cid is not None and chars.get(cid) in (KIND_SPRITE, KIND_BITMAP):
            by_id[cid] = n
    sprite_ids = sorted(c for c in by_id if chars[c] == KIND_SPRITE)
    bitmap_ids = sorted(c for c in by_id if chars[c] == KIND_BITMAP)
    ids = ",".join(map(str, sprite_ids))
    select = ",".join(f"{c}:1" for c in sprite_ids)

    def sprite_dirs(base):
        found = {}
        for d in Path(base).rglob("DefineSprite_*"):
            if d.is_dir():
                found[int(d.name.split("_")[1])] = d
        return found

    fixed_buf, fixed = fix_morph_fill_sides(decompress(Path(swf).read_bytes()))
    render_swf = out / "sheets_render.swf"  # a working copy for FFDec, deleted below
    render_swf.write_bytes(fixed_buf)
    print(f"  sheets: {fixed} morph shapes refilled for FFDec")
    origins = {}
    with tempfile.TemporaryDirectory(dir=out) as tmp:
        run_ffdec(java, ffdec, ["-format", "sprite:svg", "-select", select, "-selectid", ids,
                                "-export", "sprite", tmp, str(render_swf)], log)
        for cid, d in sprite_dirs(tmp).items():
            head = (d / "1.svg").read_text(encoding="utf-8", errors="replace")[:2000]
            m = re.search(r'<g transform="matrix\(([^)]*)\)"', head)
            vals = [float(v) for v in m.group(1).split(",")] if m else [1, 0, 0, 1, 0, 0]
            mw = re.search(r'width="([0-9.]+)px"', head)
            mh = re.search(r'height="([0-9.]+)px"', head)
            size = (math.ceil(float(mw.group(1))) if mw else 0, math.ceil(float(mh.group(1))) if mh else 0)
            origins[cid] = (-vals[4], -vals[5], size)
    for s in scales:
        dest = root / f"{s}x"
        dest.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=out) as tmp:
            run_ffdec(java, ffdec, ["-format", "sprite:png", "-zoom", str(s), "-select", select, "-selectid", ids,
                                    "-export", "sprite", tmp, str(render_swf)], log)
            for cid, d in sprite_dirs(tmp).items():
                shutil.move(str(d / "1.png"), str(dest / f"{by_id[cid]}.png"))
    render_swf.unlink()
    lines = []
    for c in sprite_ids:
        ox, oy, (w, h) = origins.get(c, (0.0, 0.0, (0, 0)))
        lines.append(f"{by_id[c]} {ox + 0.0:g} {oy + 0.0:g} vector {w} {h}")
    if bitmap_ids:
        dest = root / "1x"
        dest.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=out) as tmp:
            run_ffdec(java, ffdec, ["-selectid", ",".join(map(str, bitmap_ids)), "-export", "image", tmp, str(swf)], log)
            for f in Path(tmp).rglob("*"):
                if f.is_file() and f.suffix.lower() in (".png", ".jpg"):
                    cid = int(f.stem.split("_")[0])
                    if cid in by_id:
                        shutil.move(str(f), str(dest / f"{by_id[cid]}{f.suffix.lower()}"))
                        lines.append(f"{by_id[cid]} 0 0 bitmap 0 0")
    (root / "index.txt").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"  sheets: {len(sprite_ids)} vector x {len(scales)} scales, {len(bitmap_ids)} bitmaps")


def export_levels(swf, out, java, ffdec, chars, classes):
    """Decode the level XML that the game stores as pixels of a palette bitmap (LevelData.prepareXML).

    The colour -> character tables are read from the owner's SWF (class papaGame.data.LevelData, decompiled
    by FFDec) at extraction time, so they never live in this repository. Writes levels.xml.
    """
    log = out / "ffdec.log"
    with tempfile.TemporaryDirectory(dir=out) as tmp:
        run_ffdec(java, ffdec, ["-config", "autoDeobfuscate=1", "-selectclass", "papaGame.data.LevelData",
                                "-export", "script", tmp, str(swf)], log)
        found = list(Path(tmp).rglob("LevelData.as"))
        if not found:
            print("  levels: papaGame.data.LevelData not found, skipped")
            return
        src = found[0].read_text(encoding="utf-8", errors="replace")
    m_name = re.search(r'encodedBitmapName:String = "([^"]+)"', src)
    m_colors = re.search(r"colorTable:Array = \[([^\]]*)\]", src)
    m_texts = re.search(r"textTable:Array = \[(.*?)\];", src)
    if not (m_name and m_colors and m_texts):
        print("  levels: LevelData tables not recognised, skipped")
        return
    colors = [int(v) for v in m_colors.group(1).split(",")]
    texts = json.loads("[" + m_texts.group(1).replace("\\'", "'") + "]")
    cid = classes.get(m_name.group(1))
    if cid is None or chars.get(cid) != KIND_BITMAP:
        print(f"  levels: bitmap {m_name.group(1)} not found, skipped")
        return

    buf = decompress(swf.read_bytes())
    _, pos = read_rect(buf, 8)
    pixels = None
    for code, body in iter_tags(buf, pos + 4, len(buf)):
        if code in (20, 36) and struct.unpack_from("<H", body, 0)[0] == cid:
            fmt = body[2]
            w, h = struct.unpack_from("<HH", body, 3)
            if fmt == 3:  # palette: RGB(A) table, then row-padded indices
                n = body[7] + 1
                raw = zlib.decompress(body[8:])
                step = 3 if code == 20 else 4
                pal = [(raw[step * i + (0 if step == 3 else 1)] << 16) | (raw[step * i + (1 if step == 3 else 2)] << 8)
                       | raw[step * i + (2 if step == 3 else 3)] for i in range(n)]
                data = raw[step * n:]
                stride = (w + 3) & ~3
                pixels = [pal[data[y * stride + x]] for y in range(h) for x in range(w)]
            elif fmt == 5:  # 32-bit (A)RGB
                raw = zlib.decompress(body[7:])
                pixels = [(raw[4 * i + 1] << 16) | (raw[4 * i + 2] << 8) | raw[4 * i + 3] for i in range(w * h)]
            break
    if pixels is None:
        print("  levels: unsupported level bitmap format, skipped")
        return
    index = {c: i for i, c in enumerate(colors)}
    text = "".join(texts[index[p]] for p in pixels if p in index and index[p] < len(texts))
    (out / "levels.xml").write_text(text, encoding="utf-8")
    print(f"  levels.xml: {len(text):,} characters")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--swf", required=True)
    ap.add_argument("--out", required=True)
    ap.add_argument("--scales", default="1,2")
    ap.add_argument("--skip-ffdec", action="store_true", help="only rewrite symbols.bin and manifest.json")
    ap.add_argument("--sheets", help="file listing the classes to bake as sheets (e.g. tools/sheets/pl2.txt)")
    ap.add_argument("--only-sheets", action="store_true", help="skip shapes and sounds, only bake sheets")
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
        export_levels(swf, out, args.java, args.ffdec, chars, classes)
        if not args.only_sheets:
            export_media(swf, out, scales, args.java, args.ffdec)
        if args.sheets:
            names = [ln.strip() for ln in Path(args.sheets).read_text(encoding="utf-8").splitlines()
                     if ln.strip() and not ln.startswith("#")]
            export_sheets(swf, out, scales, args.java, args.ffdec, names, chars, classes)

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
