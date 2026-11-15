# Progress and next steps

Read this first when resuming. Rules and commands: [AGENTS.md](../AGENTS.md).

## Done (branch `claude/hola-153774`, local only; merged into `main`. Push only to `main`, only when asked)

- Toolchain: CMake + SDL3 3.4.20; Android (Gradle, verified on the POCO X5 Pro at 120 Hz); Windows
  (`tools\build_windows.bat`, VS Build Tools 2026).
- Extractor `tools/swf2assets.py` (output in `C:\Users\alfre\Proyectos\datos-papas\build\pl2`, never in the repo):
  `symbols.bin`, shapes and sheets at 1x/2x (sheet index has origin and logical size), sounds, and
  **`levels.xml`** (the level XML the game hides in the pixels of `levelMasterBitmap`; decode tables are read
  from the SWF at extraction time).
- Engine: fixed 30 Hz logic, interpolated rendering, FPS cap (original/60/120/max), timeline player, sheet
  `blit()` = `copyPixels` (clipped to the sheet's logical BitmapData size), composed tilesheets, DEV
  overlay/viewer/blit demo, `--capture <png>`, `--room N`, `--cam x,y`.
- **Milestone A done on Windows:** the default app mode "level" shows PL2 level 0 (Leaf Landing, 4 rooms) with
  sky, parallax backdrop and tiles; free camera (arrow keys / drag) clamped to the `GameCam` bounds; `[` `]` or a
  tap on the left/right half switches rooms; top band / Tab cycles level → blit demo → viewer.
- Game code (`src/game/`): `as3.h`; `papaGame::data::{LevelData, WorldData, ScreenData, DataManager}`,
  `papaGame::managers::BitmapManager`, `papaGame::display::{GameCam, GameDisplay}` (subsets) and `obf_9G::obf_4R`
  (the game object: `obf_6V` = start level, `obf_4X` = room switch).

Notes:

- QUIRK kept: `GameCam.screenbounds_maxx/maxy` are NaN (the original reads `stageWidth` from a plain MovieClip).
- Port: the visible tiles are drawn each frame into a texel-exact layer (stands in for the pre-rendered
  `tileBMP`), then placed at the sub-pixel camera offset. Faint lines at some tile edges are in the baked art
  (anti-aliased cell edges); the original composes tiles the same way, so they are expected.
- Interpolation ON also unsnaps the camera (smooth scrolling); OFF = whole stage pixels, like the original.
- `trainX` (level 9's moving background) advances once per logic frame (`GameDisplay::advanceFrame`).

- **In-app importer** (`src/import/`, desktop CLI `pl_import`): pick a game, choose its `.swf` (SDL file
  dialog; Android document picker), convert on a worker thread with progress. Native SWF reading (stb zlib),
  `symbols.bin` byte-identical to the Python tool, `levels.xml` from the game's own ABC (LevelData tables, scanner
  follows the obfuscator's jumps/constant branches), shapes + sheets rasterised with PlutoVG (fills, gradients,
  bitmaps, strokes, masks; shapes on all cores). PL2: 44 s on PC (Release). PL3 levels decode too.
  Sheet logical sizes come from shape bounds (FFDec's SVG sizes differed by 1-5 px). Sounds not converted yet.

## Next

1. Importer verified on the POCO (2026-11-14): PL2 imported from Download in about 45 s, level runs at 120 FPS.
   The old adb-pushed data is in `files/data/pl2_adb_old` (can be deleted).
2. Milestone B (below).

## Milestone B — the player (in progress)

Findings (PL2 `papaGame.managers.CharacterBlitter`, `customer*MC`):

- `createPlayerSprite` builds 5 puppet models (`customerOneSwingMC` etc. by `CustomerData.getCustomerType`,
  scale 0.27, `GlowFilter(0x000000, 1, 1.3, 1.3, 4.68)` = thin black outline) and dresses them: into the named
  containers (`body`, `head`, `eyes`, `mouth`, `neck`, `hair`, `back_hair`, `front_shoe`, `back_shoe`,
  `fronthand`, `backhand`, `front_upperarm`, `back_upperarm`, `front_forearm`, `back_forearm`,
  `cross_backforearm`, `cross_back_hand`, `weapon`, `cross_back_weap`, `glider`) it adds
  `customer_<Name>[2|3]_<part>` / `weapon_<clip>` / `glider_<name>` clips named "clip".
- Three phases (torso, front_shoe, back_shoe) hide other containers and draw `framesNeeded` (e.g.
  `basicCycles_onehand` + `attackCycles_onehand` + `grabendCycles_onehand` + skill cycles) into sheets: cell =
  `torso_bounds`/`foot_bounds` size, `floor(8191 / cellWidth)` per row, unflipped rows then flipped rows below
  (`flipBlitOffset`). `blitFrame` calls `gotoAndStop` 5 times per model (QUIRK: each call runs that frame's
  script; the last one wins), draws, repeats.
- Puppet frame scripts only do `MovieClip(this.<part>.getChildAt(0)).gotoAndStop(n)`, `stop()`,
  `gotoAndPlay("<label>")`: converted to data by the importer (`framescripts.txt`), never to C++.
- Default character: `UserData.selectedCharacter` 0 = Marty (`CustomerMarty`: clip "Marty", weapon "spatula",
  swing1, no skill), style 1.

Done: (1) importer writes `framescripts.txt`; (2) `MovieClip` named/attached children, visibility, scripts;
(3) `CharacterBlitter` draws Marty's torso/foot sheets into render targets (`--player-sheet <prefix>` saves
them; `draw()` ignores the model's GlowFilter, as Flash does); (4) `CustomerData` with Marty and Rita.
(5) `GameDisplay.reblitMultispritePlayer` draws Marty (back foot, torso, front foot) at the room's start point
with the stand cycle (`CustomerChar` SKELETON); the DEV camera starts on him. Next: `Character`/`PlayerChar`
movement, gravity and tile collisions (he starts in the air: the original drops him in from a portal),
`GameControls`, `GameCam.adjustCamera`.
(6) importer writes `datamanager.json`: every constant field of `papaGame.data.DataManager` (tile collision /
thru / grab arrays, slope tile lists, `COLLISION_TILE_NUMBER`...), read from the ABC (scanner now tracks local
registers); checked against the decompiled source: 55/55 equal. Loaded by `DataManager::loadTables`;
`getPropValue`, `setupCollisionArrays` and `MapManager` (`getTileProperty` without "door", `isWithinBounds`) are
translated. (7) `Character`: `getMyCorners`, cloud/ladder/edge checks, `fallChar`, `jumpChar`, `moveChar`
(slopes, moving tiles, crawl gaps, doors = SKELETON); `CustomerChar` derives from it (`defineVars`); the app tick
runs the player part of `detectKeys` (fall/jump) and Marty falls and lands on the ground. DEV camera follows him
(Shift/drag = free pan). Next: `Character.checkForSlopes`/`checkForCeilingSlopes`, `GameControls.detectKeys`/`keyCheckPlayer`, `CustomerChar` animation, `adjustCamera`.
Note: flipped frames are the TOP half of each sheet, unflipped below (`flipBlitOffset`).
Steps were: (1) importer writes `framescripts.txt` from the ABC; (2) `engine::MovieClip`: named children, attached
children, visibility, `gotoAndStop` running frame-script data; (3) `CharacterBlitter` into render targets with
the outline; (4) `CustomerData`/`CustomerDataFile`/`CustomerMarty`; (5) `reblitMultispritePlayer` + standing
player at the room's `startPoint`; then `Character`/`PlayerChar`/`CustomerChar` movement and collisions,
`GameControls` (actions: keyboard, gamepad, touch), `GameCam.adjustCamera`.

## Later

Enemies, objects, items, HUD/menus (needs masks/filters/text → SDL_GPU renderer), sound, widescreen, options
menu, translations, touch-control editor, PL3 (shares the engine), PL1 (AS2, separate).
