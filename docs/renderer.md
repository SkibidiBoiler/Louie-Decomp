# How the original draws, and how the port will

Notes from reading the decompiled `papaGame.display.GameDisplay`, `papaGame.managers.BitmapManager` and
`papaGame.managers.CharacterBlitter` of Papa Louie 2 (PL3 shares the same engine). No original code here.

## The original (PL2/PL3)

**Gameplay is a software blitter, not the Flash display list.**

- One 700×416 `BitmapData` framebuffer (`GameDisplay.screenBMP`) shown through a single `Bitmap` inside the HUD.
- Every frame `redrawScreen()` repaints it in a fixed order: sky → backdrop → tiles → items (back) → effects
  (back) → objects (back) → items → enemies (back) → bullets (back) → player → objects (front) → enemies (front)
  → effects (front) → items (front) → bullets (front) → effects overlay → transition/iris.
- Almost everything is `copyPixels(sheet, rect, point)`: a grid cell (`frame % perRow`, `frame / perRow`, cell
  size) pasted at an **integer** screen position (`floor(worldPos - camera - cell/2)`). No rotation or scaling
  per sprite. Facing left uses a second block of rows in the same sheet (`flipBlitOffset`), drawn mirrored
  when the sheet was built.
- The player (a customer) is a **multisprite**: back foot, torso and front foot sheets, blitted in that order
  with per-animation torso offsets.
- A handful of effects/objects draw a `MovieClip` straight onto the framebuffer with `BitmapData.draw`.

**Sheets are rasterised from vector MovieClips at load time, at 1×.**

| sheet | built by | source |
|---|---|---|
| tiles (1984×1312, 32 px tiles) | `BitmapManager.createTilesheet` | `tilesheetN_<part>` clips stacked vertically |
| enemies, background, middleground | `BitmapManager.loadBitmapFromMovieClip` | frame 1 of `enemysheet_*`, `*_bg`, `*_mg`, … |
| player / caged customers | `CharacterBlitter` | the `customer*MC` puppet, dressed by code, drawn once per animation frame (and mirrored) into torso/foot sheets |

**Menus, HUD and screens use the normal Flash display list** (`papaGame.screens`: ~265 `addChild` calls),
with text fields, 16 filter assignments and 7 code-set masks.

### Where the hard features are (PL2)

| group | classes | masks in subtree |
|---|---:|---:|
| `tilesheet*` | 35 | 13 008 |
| `enemysheet_*` | 39 | 1 160 |
| `object_*` | 30 | 357 |
| other (UI, effects…) | 285 | 366 |
| `customer*` puppets | 999 | **0** |

## The port

The split above decides the renderer:

1. **Static sheets are baked offline at N×** (`tools/swf2assets.py`, rendered by FFDec, which already
   handles masks, filters and colour transforms exactly). This replaces `BitmapManager`'s 1× runtime
   rasterisation and gives sharp graphics at any resolution scale. Each sheet records its origin offset, like
   shapes do.
2. **Player sheets are composed at runtime** by the port's own `MovieClip` renderer into render-target
   textures at N×, replacing `CharacterBlitter`. The puppets have no masks, so the current SDL renderer
   is enough.
3. **Gameplay drawing becomes textured quads**: one `blit(sheet, cell, x, y)` per original `copyPixels`, in the
   original order, in stage units (cell rects × N on the texture). This allows:
   - **high frame rates**: draw at positions interpolated between logic ticks instead of `floor()`ed ones
     (option; off = exact original positions);
   - **widescreen**: the framebuffer width (700) and visible tile count (23) are parameters of the camera,
     not of the art;
   - **resolution scale**: sheet scale N chosen from the display.
4. **UI screens** render through the `MovieClip` renderer every frame. Their few masks, filters and text need
   a renderer with stencil and shaders (SDL_GPU) later; until then masks are drawn unclipped.
