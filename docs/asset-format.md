# Asset format (v1)

Produced by `tools/swf2assets.py` from the owner's SWF, loaded by `src/engine/`. Never committed.

```
<data dir>/<game>/            e.g. data/pl2
├── manifest.json             summary: stage size, fps, available scales, counts
├── symbols.bin               characters, shape bounds, class names, timelines
├── shapes/<N>x/<id>.png      shape <id> rasterised at scale N (FFDec)
├── sheets/index.txt          one line per sheet: "<class> <origin x> <origin y> vector|bitmap"
├── sheets/<N>x/<class>.png   vector sheet: frame 1 of the class, rendered at scale N (FFDec)
├── sheets/1x/<class>.png|jpg bitmap sheet (a linked BitmapData), as stored in the SWF
└── sounds/<id>_<name>.*      embedded sounds (FFDec)
```

**Sheets** are the bitmaps the original builds at runtime (see `docs/renderer.md`); the list per game is in
`tools/sheets/<game>.txt`. A sheet PNG's top-left pixel is the point `(origin x, origin y)` of the symbol, so
the original's `copyPixels(sheet, Rectangle(sx, sy, w, h), ...)` reads texture pixels
`((sx - origin x) * N, (sy - origin y) * N)` with size `(w * N, h * N)`. Tilesets are composed at load time
from their five parts (`<tileset>_solid|soft|bridge|blocks|details`), stacked and clipped as in
`BitmapManager.createTilesheet`.

A shape PNG at scale N covers the shape's bounds: its top-left corner is `(x, y)` from `symbols.bin` and its
size is `ceil(w*N) x ceil(h*N)` pixels, so it is drawn over `x .. x + png_width / N` in shape units (pixels at 1x).

## symbols.bin

Little-endian. `str` is a `u32` index into the string table; `0xFFFFFFFF` means "none".

```
char[4]  magic "PLSY"
u32      version = 1

u32      string count, then per string: u16 byte length, UTF-8 bytes

u32      character count, then per character:
           u16 id, u8 kind          1 shape, 2 sprite, 3 morph shape, 4 static text, 5 edit text,
                                    6 button, 7 bitmap, 8 sound, 9 font
           kind 1 only: f32 x, y, w, h   (bounds in pixels)

u32      class count, then per class: str name, u16 character id      (AS3 SymbolClass linkage)

u32      timeline count, then per timeline (id 0 = main timeline):
           u16 id, u16 frame count
           u16 label count, then per label: str name, u16 frame (0-based)
           u32 track count, then per track:
             u16 depth, u16 character id, str instance name, u16 clip depth (>0: this track is a mask
             up to that depth), u8 blend mode, u8 filter count, u16 first frame, u16 last frame (inclusive),
             u32 key count, then per key:
               u16 frame, u8 flags, f32 a, b, c, d, tx, ty      (child -> parent: x' = a x + c y + tx,
                                                                                  y' = b x + d y + ty)
               flags & 1: f32 mult r, g, b, a, add r, g, b, a   (add terms in 0..1)
               flags & 2: u16 morph ratio
```

A **track** is one persistent child instance: the same character at the same depth from its first to its last
frame. Its state at frame `f` is the last key with `key.frame <= f`. When the playhead moves, children whose track
stays alive keep their own state (and, for sprites, their own playhead); others are created or destroyed.

## framescripts.txt

Timeline frame scripts converted to data by the importer (`src/import/abc.cpp`, `Abc::frameScripts`). One
statement per line, in script order: `<class> <frame (0-based)> <op> <target> <number>`, where `op` is `stop`,
`play`, `gotoAndPlay`/`gotoAndStop` (target = label or `-`, number = 1-based frame) or `child` (target = container
instance name; `MovieClip(this.<target>.getChildAt(0)).gotoAndStop(<number>)`). Statements the converter does not
recognise are skipped and counted in the import log.

## datamanager.json

Constant fields of `papaGame.data.DataManager` (instance initialiser and static constants) as one JSON object,
`{ "<field>": <number | string | array> }`, read from the game's bytecode by the importer so these tables (tile
collision and slope lists, special tile numbers, ...) never live in the repository.

## customerchar.json

The same for `papaGame.models.characters.CustomerChar` (the player): its animation cycles
(`"<name>CycleFrames": [name, type, blitStyle, randomize, loopFrame, [frames]]`, torso and `foot_` cycles),
`torsoOffsetsX/Y`, and the attack rectangles and frame ranges (`"swing_attackRect": [x, y, width, height]`,
`"swing_frameRange": [start, end, repeat]`). A `new Rectangle(...)` with constant arguments is written as its
four numbers.
