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
(Shift/drag = free pan). (8) Controls: `engine::Input` (actions from keyboard: arrows, Up = jump like the
original, Space = attack, plus WASD/Z/X; gamepad; touch) and `TouchControls` (translucent round buttons in window
pixels: left/right/down bottom-left, jump/attack bottom-right; shown on Android, `--touch` on PC). `GameControls`
(`detectKeys` player part, `keyCheckPlayer` walk/turn/duck/air control, `jumpPlayer` jump/short jump/coyote time)
and a SKELETON `CustomerChar::updateObject` (stand/run/jump/fall/turn/duck cycles). DEV taps moved to the top
band (left FPS, middle mode, right interpolation). `--walk-test` walks and jumps for captures. Marty walks and
jumps. (9) `Character.checkForSlopes` / `checkForCeilingSlopes` translated (`onSlope` is the slope type 1-10,
not a flag); Marty walks up and down the hills. (10) `GameCam.adjustCamera` translated (barge bob left out:
`isOnBarge()` is always false in PL2; `jiggleCamera` and `cameraAnchor` are SKELETON); called at the end of
`detectKeys` and with snap from `§_-52§` (start level, reached directly from `obf_6V` until the intro exists).
DEV: Shift+arrows / mouse drag pan; `--cam` and room switches leave the camera free until the player moves.
(11) Attack and the real player animation: `CustomerChar.updateObject` translated (skid build-up and decay,
stun, lunge, walk speed on slopes, `checkIfAttacking`, `fallChar` with `lastSafeTime`; sliding, pounding,
gliding, wall jumps, hitting enemies/objects, items and doors are SKELETON), `updateSprite` with the torso and
foot cycles (`grabAnimationCycle` / `grabFootAnimationCycle`, frame sync, `endAnimationCycle`),
`startAttacking` / `setupAttackRect` / `cancelAttack`, and the attack, skid and lunge parts of `keyCheckPlayer`.
The cycles and attack tables come from `customerchar.json` (importer; re-import needed for older data).
`--walk-test` also attacks at frames 82-84.
(12) Ladders and doors: `PlayerChar.checkIfOnLadder` and the ladder part of `keyCheckPlayer` (grab, climb at
climbSpeed 5, hop off the top); `MapManager.setProperty` (ladders are `GameObject3` objects that mark tiles, so
real ladders appear with objects; DEV `--ladder x,y,h`). Doors: `ScreenData.checkForDoorID`,
`WorldData.getDoorArray`, `MapManager.getTileDoor` (= getTileProperty "door"), `checkIfOnDoor`,
`checkGoingThroughDoor`, `enterDoor`, `GameControls.travelThroughDoor`, the room wipe (`startTransition`,
`blitTransition`, `finishTransition`) and `§_-4X§` placing the player at the target door.
DEV testing without screenshots: `--input "R1-60,U61-90"` (keys LRUDJA, logic frames), `--trace` (one line per
frame: room, position, tile, cycles, flags; plus the room's doors and ladder tiles), `--frames N`, `--level N`.
`tools/as_func.py <deob dir> <Class> <method>` prints one original function compactly.
(13) Enemies: `EnemyManager` (§_-E§: addEnemy, processEnemies, clearEnemies, saveEnemies, buildEnemies,
removeEnemy with a one-frame graveyard, getEnemy, preblit), `Enemy` (placeEnemy, getHit, killMe juggling,
lookForPlayer, adjustSpeedForSlopes, saveEnemyKill...), and Leaf Landing's types: `Enemy1` onion, `Enemy2` brown
onion (paces to targetTileX), `Enemy6` tomato (hops, QUIRK: delay timer never resets), `Enemy34` Burgerzilla.
`Character` now holds the shared animation (`updateSprite`, `spriteChangeFrame`, `cycleFrames`), `Rect`,
health, sprite collision and proximity sound helpers. Cycle tables come from `enemies.json` (importer).
`DataManager.initializeScreenObjects` builds the room's enemies (start of level and after each door);
`ScreenData.saveEnemyState` remembers kills; `GameDisplay.reblitEnemies` draws them (interpolated); `UserData`
points / kill tallies. `--trace` adds an `E` line per frame (id:type x,y cycle, d/s/h flags).
SKELETON: the other 35 enemy types (`createEnemy` returns null: logged "ENEMY DOESN'T EXIST"), multi-sprite
enemies, effects (dizzy stars, starburst), sounds, triggers to objects.
(14) Hitting enemies and player damage: `CustomerChar.checkForHittingEnemies` (attack rectangle during the attack
frames, slide, ground pound, stomping stuns and bounces, touching hurts or shoves; `isFinalBlow`), combo
(`addToCombo`, `updateComboTimer`, `clearCombo`), `PlayerChar.canGetHit`, `hurtPlayer` (knock-back stun, or death
when the health, 3, runs out), `shovePlayer`, `updateStunTime`, `restartPlayer` + `UserData.adjustLives`
(PL2 starts with 0 lives, so the first death ends the level), falling off the level (`fellOffLevel`), the
stand/jump/duck collision rectangles and the rest of `startupPlayer` (start-up stun = invincibility flicker).
`GameControls.detectKeys` now moves hit, shoved and dead players like the original.
SKELETON: the game over screen and iris (a death restarts the level), camera jiggle, effects and sounds.
(15) Objects: `ObjectManager` (§_-7u§: addObject, processObjects, clearObjects, saveObjects, buildObjects,
removeObject, getObject; `checkSteppingOnObjects` only the moving-tile part), `GameObject` (placeObject,
add/removeCollision through `MapManager.set/unsetProperty`, activate / deactivate cycles, acceptsDamage) and the
types `GameObject3` ladder (marks its tiles; `GameDisplay.blitLadder`), `GameObject21`-`25` cracker blocks (solid,
broken by ground pounds), `GameObject28` checkpoint (`DataManager.saveCheckpoint` is a log line for now).
`CustomerChar.checkForHittingObjects` / `checkForPoundingObjects`; `GameDisplay.reblitObjects`. The base object's
cycles come from `objects.json` (importer). The DEV `--ladder` flag is gone: real ladders work (level 1).
SKELETON: the other object types (cage, pulley, blimps, berries, switch, door, boulder, cannon, sauce, spikes,
training sensor), events/triggers between objects, carrying, pushing, stepping on objects, CheckpointData.
(16) Items: `ItemManager` (§_-3Z§: addItem, checkForCollecting, processItems, saveItems -> `ScreenData.updateItems`,
clearItems, buildItems, removeItem; `itemIDsToNames` from `items.json`), `GameItem` (placement, flicker, moving
items) and the nine PL2 items (`GameItem_Coin`, `_PapaCoin`, `_SargeCoin`, `_RadishCoin`, `_Heart`, `_Daisy`,
`_Balloon`, `_FizzoCan`, `_GummyWorm`). `itemArray` rows are now `as3::Value` (saved rows name their item).
`PlayerChar.getHeart` / `checkForCollectingItems`, `UserData` money / special items, `GameDisplay.reblitItems`.
The `T` trace line shows health, money and special items. SKELETON: dropMoney (it names PL3 money items),
dropped weapons, GoldHelmet, the item effects and sounds.
(17) Effects: `EffectManager` (§_-V§: addEffect, processEffects, clearEffects, removeEffect, getEffect), `Effect`
and 16 effect classes: `DizzyEffect` (stunned enemies and the hurt player, who keeps `stunStarIDs`),
`StarburstEffect` (enemy hit), `DustEffect` (landing, turning, sliding, lunging), `CrackerBreakEffect`,
`ZillaPartsEffect`, `PlayerCheckpointEffect`, `EndPortalEffect` (start of a level) and the nine `Item*Effect`
pick-ups (`§_-AF§.§_-AJ§` quadratic easing = `obf_AJ`). Cycle tables from `effects.json` (all 50 effect classes).
`GameDisplay.reblitEffects`; `--trace` adds an `FX` line. Every entity now saves its previous position for the
interpolated render (effects and objects were drawn from a stale position).
Bullets (task 6) wait for the shooting enemies and the gun customers.
SKELETON: the other 34 effect classes.
(18) Text: the importer bakes every embedded font into a glyph atlas and writes the static/edit texts to
`text.json` (`Rasterizer::exportText`); `engine::TextLibrary` draws them, `MovieClip` draws Text/EditText
children and `MovieClip::setText(instance, text)` sets a field (`.text` / `.htmlText`). DEV `--view <class>`
shows a symbol in the viewer (`--view hudMC` shows the HUD with its real fonts).
(19) HUD: `GameHUD` (§_-6Z§: setupHUD with `hudMC`, updateDisplay, updateTimer, show/hideHUD,
updatePlayerHealth; drawn over the game in stage pixels), `§_-AF§.§_-81§` number/time formatting (`obf_81`),
`ChallengeManager` (§_-94§) with its `challenges` / `badges` tables from `challenges.json` and
`getChallengeTargetAmount`; `UserData` refreshes the HUD (UserData.cpp). The importer's initialiser scanner now
reads object literals, `new Class(args)` (as {"new", "args"}) and static constants of other classes
(`Challenge.RESCUE`); bogus values it used to pick up from unread getproperty results are gone.
`MovieClip::setChildAlpha` for `.alpha` on text fields. SKELETON: HUD buttons (menu, mute, pause), portrait,
challenge bubbles, the challenge recording.
Sound (task 9) is BLOCKED: every DefineSound in PL2 is MP3 (94 at 22 kHz, 21 at 11 kHz, 5 at 44 kHz, 1.5 MB) and
the project has no MP3 decoder (SDL3 and stb have none). Adding one (minimp3 or dr_mp3, public domain) means
downloading third-party code: waiting for the owner's permission. Then: importer decodes DefineSound to WAV/PCM
(sounds/<name>.wav), `SoundManager` (§_-7C§: playSound with volume/pan, playTrack, crossfade, endMusic, mute)
on SDL3 audio streams. Until then every playSound call is a `// SKELETON:` comment.
Next: the level start/end (task 10). How the original ends a level (read so far):
1. `GameObject1` (cage pulley, `isGrabRideable`): the player grabs it (`PlayerChar.checkForGrabbingObjects` /
   `grabRideableObject`), rides it down (`grabYextraOffsets`), it deactivates and `broadcastTriggered`s through
   the room's events (`EventManager` §_-7t§ `loadEvents` gives objects `tellTriggerArray`) to `GameObject19` cage
   (`GameObject20` = flipped cage).
2. `GameObject19.deactivateObject`: opens the cage effects (`CageFrontBars`/`CageBackBars`/`CageSpinner`/
   `CageCustomerEffect`, the trapped customer drawn as a puppet), unlocks the customer (`UserData.unlockCustomer`,
   `CustomerData.getTrappedCustomerIndex(§_-8W§, index)`), sets `finishTargetX` and calls
   `PlayerChar.finishLevel("cage")`.
3. `CustomerChar.updateObject` finishing branch (CustomerChar.as 1179-1290): walk to `finishTargetX`, jump
   `finishJumpsMax` times (the customer effect jumps too), `EndPortalEffect` beside him, wait, iris out
   (`GameDisplay.startMainIrisOut`, a masked clip: needs mask support), fly into the portal, `flashPortal`.
4. `GameDisplay.redrawScreen`: when the iris clip ends, `gameObj.finishLevel()` (or §_-2K§ game over, §_-99§
   reset after dying in an elevator, §_-14§ outro after level 99).
Masks (clip depth) are not drawn yet: needed for the iris and many menus.
Note: flipped frames are the TOP half of each sheet, unflipped below (`flipBlitOffset`).
Steps were: (1) importer writes `framescripts.txt` from the ABC; (2) `engine::MovieClip`: named children, attached
children, visibility, `gotoAndStop` running frame-script data; (3) `CharacterBlitter` into render targets with
the outline; (4) `CustomerData`/`CustomerDataFile`/`CustomerMarty`; (5) `reblitMultispritePlayer` + standing
player at the room's `startPoint`; then `Character`/`PlayerChar`/`CustomerChar` movement and collisions,
`GameControls` (actions: keyboard, gamepad, touch), `GameCam.adjustCamera`.

## Later

Enemies, objects, items, HUD/menus (needs masks/filters/text → SDL_GPU renderer), sound, widescreen, options
menu, translations, touch-control editor, PL3 (shares the engine), PL1 (AS2, separate).
