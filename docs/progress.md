# Progress and next steps

Read this first when resuming. Rules and commands: [AGENTS.md](../AGENTS.md).

## Done (branch `claude/hola-153774`, PR #1)

- Toolchain: CMake + SDL3 3.4.20; Android (Gradle, verified on the POCO X5 Pro at 120 Hz); Windows
  (`tools\build_windows.bat`, VS Build Tools 2026).
- Extractor `tools/swf2assets.py` (output in `C:\Users\alfre\Proyectos\datos-papas\build\pl2`, never in the repo):
  `symbols.bin`, shapes and sheets at 1x/2x (sheet index has origin and logical size), sounds, and
  **`levels.xml`** (the level XML the game hides in the pixels of `levelMasterBitmap`; decode tables are read
  from the SWF at extraction time).
- Engine: fixed 30 Hz logic, interpolated rendering, FPS cap (original/60/120/max), timeline player, sheet
  `blit()` = `copyPixels`, composed tilesheets, DEV overlay/viewer/blit demo, `--capture`.
- Game code started (`src/game/`): `as3.h` helpers, `papaGame::data::LevelData`, `WorldData`, `ScreenData`
  translated and compiling (not used yet).

## Milestone A — scroll through PL2's first room (in progress)

Findings (from the decompiled PL2 code in `datos-papas\extracted\pl2-deob\`):

- Level 0 = world with `<tileset>0</tileset>` = "Leaf Landing". `DataManager.setupLevelScreens(level)` picks the
  world whose `tileset` text equals the level number, builds a `ScreenData` for every id in
  `WorldData.getWorldRoomIDs()` (field 6 of each `roomArray` row) and starts at `worldStartRoom`.
- `BitmapManager.tilesetNames/bgNames/mgNames[level]` → `"tilesheetN"`; tiles = composed tilesheet (62 tiles
  per row, 32 px), sky = `<name>_bg`, backdrop = `<name>_mg`.
- Tiles: index v → sheet cell `(v % 62, floor(v / 62))`; every layer of a cell drawn in order. The original
  pre-renders the room into 2880-px strips (Flash bitmap limit) and copies the visible part; the port draws the
  visible tiles directly (same result).
- `GameDisplay.defineScreenBounds`: `currentXcoord = floor(cam.x - floor(23/2)*32)`, same for y with 13 rows.
- `reblitSky`: fill 0xFFCC00, then bg sheet wrapped horizontally at `round(currentXcoord / 8)`.
- `reblitBackdrop` ("scroll", speeds 3/3): mg sheet wrapped at `round(x/3)`; vertical position
  `floor(min(1, y / max(levelPxHeight - 416, 416)) * (levelPxHeight - mgHeight) - y)`.
- `GameCam.setCameraBounds`: `min_x = (12-1)*32`, `min_y = (7-1)*32`, `max_x = levelW - 350`,
  `max_y = levelH - 208 - 16`; initial camera 240, 672.
- Game object is obfuscated `§_-4R§` → C++ `obf_9G::obf_4R`; fields `§_-71§` GameDisplay, `§_-3Y§` GameCam,
  `§_-AQ§` DataManager, `bitmapManager`, `§_-6Z§` GameHUD, `playerObj`, `§_-8o§` GameControls.

Next steps:

1. `src/engine/sheets.*`: read the new index columns (`... vector|bitmap <width> <height>`), store the logical
   size in `Sheet`, clip blits to `0..width x 0..height` (bitmaps: texture size).
2. Translate `papaGame::data::DataManager` (subset: `setupLevelScreens`, `setActiveRoom`, `clearLevelScreens`),
   `papaGame::managers::BitmapManager` (subset: tile/bg/mg sheets via `SheetLibrary`), `papaGame::display::GameCam`
   (fields, `setCameraBounds`; DEV free pan clamped to the bounds, prev position for interpolation) and
   `papaGame::display::GameDisplay` (`defineScreenBounds`, `reblitSky`, `reblitBackdrop`, `reblitTiles`, the
   first lines of `redrawScreen`), plus `obf_9G::obf_4R` holding them.
3. App: new default mode "level" (load `levels.xml`, level 0, room 0; drag/arrow keys pan the camera;
   `[`/`]` previous/next room); keep blit demo and viewer behind the top-band tap.
4. Verify on Windows (`--capture`) and on the POCO (push `levels.xml` and the re-baked `sheets/` too).

## Milestone B — the player

`CharacterBlitter` (compose torso/foot sheets from `customer*MC` at runtime with `MovieClip` into render
targets), `Character`/`PlayerChar`/`CustomerChar` movement and tile collisions, `GameControls` with an action
layer (keyboard, gamepad, touch; later the touch-layout editor), `GameCam.adjustCamera` following the player.

## Later

Enemies, objects, items, HUD/menus (needs masks/filters/text → SDL_GPU renderer), sound, widescreen, options
menu, translations, touch-control editor, PL3 (shares the engine), PL1 (AS2, separate).
