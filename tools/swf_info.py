#!/usr/bin/env python3
"""Summarise a SWF file: header, ActionScript version, code size and asset counts.

Read-only and local: it never modifies or uploads the file. Usage:
    python -I tools/swf_info.py path/to/game.swf
"""
import hashlib
import lzma
import struct
import sys
import zlib
from collections import Counter

TAG_NAMES = {
    0: "End", 1: "ShowFrame", 2: "DefineShape", 6: "DefineBits", 9: "SetBackgroundColor",
    10: "DefineFont", 11: "DefineText", 12: "DoAction", 14: "DefineSound", 18: "SoundStreamHead",
    20: "DefineBitsLossless", 21: "DefineBitsJPEG2", 22: "DefineShape2", 26: "PlaceObject2",
    32: "DefineShape3", 33: "DefineText2", 35: "DefineBitsJPEG3", 36: "DefineBitsLossless2",
    37: "DefineEditText", 39: "DefineSprite", 43: "FrameLabel", 45: "SoundStreamHead2",
    46: "DefineMorphShape", 48: "DefineFont2", 56: "ExportAssets", 59: "DoInitAction",
    60: "DefineVideoStream", 69: "FileAttributes", 70: "PlaceObject3", 72: "DoABC (old)",
    75: "DefineFont3", 76: "SymbolClass", 77: "Metadata", 82: "DoABC", 83: "DefineShape4",
    84: "DefineMorphShape2", 86: "DefineSceneAndFrameLabelData", 87: "DefineBinaryData",
    88: "DefineFontName", 90: "DefineBitsJPEG4", 91: "DefineFont4",
}
SHAPES = {2, 22, 32, 83, 46, 84}
BITMAPS = {6, 20, 21, 35, 36, 90}
SOUNDS = {14}
FONTS = {10, 48, 75, 91}


def decompress(data: bytes) -> bytes:
    sig = data[:3]
    length = struct.unpack_from("<I", data, 4)[0]
    if sig == b"FWS":
        return data
    if sig == b"CWS":
        return data[:8] + zlib.decompress(data[8:])
    if sig == b"ZWS":
        props = data[12]
        dict_size = struct.unpack_from("<I", data, 13)[0]
        lc, rem = props % 9, props // 9
        lp, pb = rem % 5, rem // 5
        dec = lzma.LZMADecompressor(lzma.FORMAT_RAW, filters=[
            {"id": lzma.FILTER_LZMA1, "dict_size": dict_size, "lc": lc, "lp": lp, "pb": pb}])
        body = dec.decompress(data[17:], max_length=length - 8)
        return data[:8] + body
    raise ValueError(f"not a SWF file (signature {sig!r})")


def read_rect(buf: bytes, pos: int):
    nbits = buf[pos] >> 3
    total = 5 + 4 * nbits
    raw = int.from_bytes(buf[pos:pos + (total + 7) // 8], "big")
    width_bits = ((total + 7) // 8) * 8
    vals = []
    for i in range(4):
        shift = width_bits - 5 - (i + 1) * nbits
        v = (raw >> shift) & ((1 << nbits) - 1)
        if nbits and v & (1 << (nbits - 1)):
            v -= 1 << nbits
        vals.append(v)
    return vals, pos + (total + 7) // 8


def iter_tags(buf: bytes, pos: int, end: int):
    while pos + 2 <= end:
        code_len = struct.unpack_from("<H", buf, pos)[0]
        pos += 2
        code, length = code_len >> 6, code_len & 0x3F
        if length == 0x3F:
            length = struct.unpack_from("<I", buf, pos)[0]
            pos += 4
        yield code, buf[pos:pos + length]
        pos += length
        if code == 0:
            break


def main(path: str) -> int:
    raw = open(path, "rb").read()
    print(f"file:        {path}")
    print(f"size:        {len(raw):,} bytes")
    print(f"sha256:      {hashlib.sha256(raw).hexdigest()}")
    buf = decompress(raw)
    version = buf[3]
    print(f"format:      {raw[:3].decode()} (SWF version {version}), uncompressed {len(buf):,} bytes")

    (xmin, xmax, ymin, ymax), pos = read_rect(buf, 8)
    frame_rate = struct.unpack_from("<H", buf, pos)[0] / 256.0
    frame_count = struct.unpack_from("<H", buf, pos + 2)[0]
    pos += 4
    print(f"stage:       {(xmax - xmin) / 20:g} x {(ymax - ymin) / 20:g} px, {frame_rate:g} fps, "
          f"{frame_count} root frames")

    counts = Counter()
    sprite_counts = Counter()
    as3 = None
    abc_bytes = 0
    action_bytes = 0
    abc_names, symbols, exports = [], [], []
    for code, body in iter_tags(buf, pos, len(buf)):
        counts[code] += 1
        if code == 69 and body:
            as3 = bool(body[0] & 0x08)
        elif code == 9 and len(body) >= 3:
            print(f"background:  #{body[0]:02x}{body[1]:02x}{body[2]:02x}")
        elif code in (72, 82):
            abc_bytes += len(body)
            if code == 82:
                name = body[4:body.index(0, 4)].decode("utf-8", "replace")
                abc_names.append(name or "<unnamed>")
        elif code in (12, 59):
            action_bytes += len(body)
        elif code == 39:
            for sub_code, sub_body in iter_tags(body, 4, len(body)):
                sprite_counts[sub_code] += 1
                if sub_code == 12:
                    action_bytes += len(sub_body)
        elif code in (56, 76):
            n = struct.unpack_from("<H", body, 0)[0]
            p = 2
            for _ in range(n):
                p += 2
                e = body.index(0, p)
                (symbols if code == 76 else exports).append(body[p:e].decode("utf-8", "replace"))
                p = e + 1

    if as3 is None:
        as3 = abc_bytes > 0
    print(f"actionscript: {'AS3 (AVM2)' if as3 else 'AS1/AS2 (AVM1)'}")
    print(f"code:        DoABC {abc_bytes:,} bytes in {len(abc_names)} block(s); "
          f"DoAction/DoInitAction {action_bytes:,} bytes "
          f"({counts[12] + sprite_counts[12]} frame scripts, {counts[59]} init scripts)")
    if abc_names:
        print(f"abc blocks:  {', '.join(abc_names[:10])}")

    def total(codes):
        return sum(counts[c] for c in codes)

    print(f"assets:      {counts[39]} sprites, {total(SHAPES)} shapes, {total(BITMAPS)} bitmaps, "
          f"{total(SOUNDS)} sounds, {total(FONTS)} fonts, {counts[37]} edit texts, "
          f"{counts[87]} binary blobs, {counts[60]} videos")
    if symbols:
        print(f"symbolclass: {len(symbols)} -> {', '.join(symbols[:15])}{' ...' if len(symbols) > 15 else ''}")
    if exports:
        print(f"exports:     {len(exports)} linkage names")

    print("tags:")
    for code, n in sorted(counts.items(), key=lambda kv: -kv[1]):
        print(f"  {TAG_NAMES.get(code, f'tag {code}'):30} {n}")
    return 0


if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(__doc__)
        sys.exit(2)
    sys.exit(main(sys.argv[1]))
