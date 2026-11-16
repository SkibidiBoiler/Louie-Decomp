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
(21) Sound: every DefineSound in PL2 is MP3. The owner approved minimp3 (CC0), added as the submodule
`external/minimp3`, pinned to the 2022 commit 3f50914 (the repo got a burst of new commits on 2026-11-15; not
reviewed, so not taken). The importer (`src/import/sound_export.cpp`) decodes every sound with an AS3 linkage
name to 16-bit WAV, `sounds/<linkage name>.wav` (117 files, 14 MB; names keep their own ".wav", e.g.
`getcoin.wav.wav`), skipping the MP3 SeekSamples and cutting to SampleCount so music loops cleanly.
`engine::Audio` (audio.cpp): one SDL3 audio stream per playing sound, bound to the device, fed by a callback that
applies the channel's volume, pan (Flash SoundTransform) and the master volume (SoundMixer); `SoundChannel` is
the game's handle (stop, soundTransform). `SoundManager` (§_-7C§) translated whole: playSound, playTrack with the
two music channels, switchMusic / fadeMusic (fadeTimer driven by the logic clock, 100 ms), endMusic, mute,
pauseMusic, startLevelMusic. Sound calls hooked in everything ported so far (level start, jumps, ladders, crawl
gaps, attacks, hits, stomps, hurt / death, items, cracker blocks, checkpoint, the finish and fanfare).
DEV `--mute` silences the output; `--trace` prints an `S` line with the sounds started each frame.
QUIRK: GameObject21 plays "impact_2.wav", which is not in the SWF ("Error Playing Sound").
SKELETON: Flipline API mute events, the HUD mute button, sounds of classes not ported yet.
(22) Customers and skills: all 28 `papaGame.data.customers.Customer*` classes (one header each, translated from
their initCustomerData) in `CustomerData.customersIncluded` order. `CustomerChar` now takes skillType, canPush,
jumpgravity and bulletstart from the customer, and has the skills: sliding (`startSliding`; Down on a slope,
Scooter after a double jump; speed along the slope, decay, the hop of Scooter / Kahuna, the slide sound),
ground pound (`startPounding`, Down in the air), double jump, gliding (`startGliding`, the glide boost), wall
sliding and wall jumping (`startWallJumping`, `checkIfAgainstWall`, jumping off with the jump key or the arrow
away from the wall), crawl gaps (`checkForCrawlGap`, `isInsideCrawlGap`), `stopGrabRiding`;
`GameControls.jumpPlayer` translated whole (drop from a hang, double jump / glide / wall jump branches, river /
quicksand jump speeds). DEV `--customer N` plays as customer N (0 Marty, 9 Kahuna, 11 Ninjoy, 15 Scooter,
16 Connor, 20 Greg, 26 Papa Louie); `--trace` shows dj / glide / pound / slide / wallslide / walljump.
Checked by trace: Connor's double jump, Papa Louie's glide, Kahuna's pound and slope slide with the end hop;
Kahuna drawn with his surfboard. Not checked in game: wall jumping and crawl gaps (no wall or gap in level 0's
first room).
SKELETON: pushing (needs the pushable objects), Foodini's balloons (FoodiniBalloonEffect, FoodiniGlideEffect),
the skill tutorials (§_-c§), cheese (jumpCheesed), projectile weapons (toss / pistol / longgun / bazooka need
BulletManager, task 6), character select (MainMenuScreen, with the menus).
(23) Menus, part 1. Engine: buttons (the importer exports DefineButton / DefineButton2 as four-frame timelines:
up, over, down, hit; `MovieClip` shows the state under the pointer), x / y / scale set from code
(`setX`, `setY`, `setScale`), mouse events (`onClick`, `onMouseDown`, `onRollOver`; hit test by bounding boxes,
`MovieClip::dispatchPointer`), `MovieClip::create(class)`, `createEmpty`, `removeChild`, and masks
(`MaskLayers`: the masked children and the mask go to two render targets and are combined with a custom blend;
only on the window target, so a mask inside a mask or inside a sheet is drawn unclipped). The app tracks the
pointer (mouse or finger) in stage coordinates. Translation tables: `lang/<code>.json` in the repository
(the port's own interface texts), compiled in (`engine::Lang`), language from settings.ini (`language=`) or
the system; `TextLibrary::drawString` draws them with the SWF's fonts and builds the letters the fonts lack
(á é í ó ú ñ ü ¿ ¡) from a base letter and a mark. `engine::MenuBar` stands in for the Flipline API's screen
frame (the API is not ported): the SWF's api_* panels and button pieces, the API's sizes, labels from the
tables; `§_-4R§.showApiScreen` replaces api.§_-1G§ and the bars send their events to the screens' container
(`addContainerListener`). Screens: `SplashScreen` (title, Play iris out, credits panel) and `StageIntroScreen`
(the portal intro with the customer puppet, then §_-6V§), `§_-4R§` §_-8O§, §_-Q§, §_-q§ and the container
§_-5h§. The app now starts at the title screen; any DEV level flag (--level, --room, --at, --input, --trace,
--customer) still starts straight in level 0. DEV `--lang es`, `--tap "x,y@frame;..."` (scripted taps).
Checked: title screen in Spanish (capture), Play -> iris -> stage intro (capture: portal and Marty) -> level 0
by trace. SKELETON: SlotSelectScreen and MainMenuScreen (save slots, map, character select: "SlotSelect" goes
straight to the stage intro), ScoreTallyScreen, NewCustomerScreen, the boss / outro / parade screens, the
puppet's GlowFilter outline, BitmapManager.createCagedSprites, the bars' slide-in animation.
(24) Save data and slots (task 13 and menus, part 2). `engine::SharedObject`: flash.net.SharedObject as JSON
files in <pref path>/saves/<name>.json (written through a temporary file), with `writeJson`. `UserData` translated
whole except the Flipline API calls: tallies, scores and time bonus, high scores and best times, warp coins and
world unlocks, challenges and badges records, customers and outfits, key settings, training flags, slot labels and
completion, createNewSlot / loadData / saveProgress (all its field sets) / eraseSlot. Every starting value comes
from userdata.json (createNewSlot resets to the field initialisers). QUIRKs kept: getCompletionPercentage is
always NaN (it divides by an unset variable), so hasEarnedEverything is never true; the challenge count is a fixed
49; a slot without enemyKills gets 44 zeros. `ChallengeManager`: resetAllTallies, medal progress, badge count
(recordTag & co. are empty until task 15). `DataManager`: outfitPrices, coinsToUnlockWorld, getOutfitPrice.
Screens: `SlotSelectScreen` (three slots with name, puppet and totals, delete with confirmation, new game:
Marty or Rita, then the name, typed with the keyboard or the phone's keyboard; the backup buttons and the
"missing save" widget were API features and stay hidden) and `GameIntroScreen` (the story, Skip). Flow: title ->
slots -> new game: story -> level 1 / existing slot: (SKELETON: the map) its latest area; §_-9k§ now ends in §_-8O§
as in the original, so finishing a level goes through the stage intro too. Engine: TextFields made by code
(`MovieClip::addTextField`), `mouseChildren`, `engine::makeApiButton` (the API's button, for the name entry),
`MenuBar::setTitle`, keyboard text for screens, letters built from marks in the SWF's edit texts too. Fixes: frame
scripts of timeline classes in a package (`<file>_fla.Class`) were never attached (the importer wrote the short
name; it now writes the linkage name, and the loader also matches a unique short name); `--frames` now counts every
app frame (it only counted level frames, so menu tests never ended). DEV `--capture-frame N`.
Checked: title -> slots -> New Game -> character (Marty / Rita puppets) -> name (CONTINUE) -> save written ->
story -> Skip -> stage intro -> level 1; loading the slot -> level; slot card with the saved game (captures).
SKELETON: MainMenuScreen (map, character select, outfits, badges, help, credits), ScoreTallyScreen (the level
score is not saved yet), the menus' thumbnails (generateCustomerBitmap / generateEnemyBitmap), translated static
texts of the SWF ("New Game", "Enter your name below:", task 16).
(25) Main menu (menus, part 3). `MainMenuScreen`: the map (unlocked areas with their number, the "new" ring,
the totals, the reveal animation of a newly opened area, the challenge bubble on roll-over), and before a level
the area's panel (title, number, high score, the level preview inside its mask, the six challenges with the rescued
customers' faces) and the customer choice (thumbnails of the unlocked customers, the "new" banner, name, game,
weapon, skill, the puppet that runs in while the old one runs out, outfits A/B/C with their prices, buying one).
Start Level closes the iris and starts the area (§_-8W§, §_-9k§). Sections slide as in the original
(setSection / tweenSections); the credits section works with its web links hidden. Bars: MainMenu (CREDITS, QUIT)
and MapSelectMenu (Start Level, Back to map). `CustomerData.generateCustomerBitmap` / `getCustomerBitmap`
(every customer's face drawn into a texture). `ChallengeManager.getChallengeType` / `getChallengeSkillNeeded`.
Engine: code-set masks (`MovieClip::setMask`), Bitmaps added by code (`addBitmap`), ROLL_OUT (the pointer's
targets are cleared when a clip dies), `currentFrameLabel`, removed clips are destroyed one frame later
(`collectGarbage`: a screen often removes the clip whose button handler is running). Fix: a clip now runs its first
frame's script when it is made, as Flash does (the character section's iris played on and closed itself; other
clips with a stop() on frame 1 were playing too). The HUD now shows its MENU and sound buttons (DefineButtons).
Checked (captures and taps): saved slot -> map -> area 1 -> customer choice (Marty, then Rita runs in) -> Start
Level -> stage intro -> level 0 as Rita; level end regression.
SKELETON: the baddies, badges, controls, help, info (pause menu) and confirm-quit sections and their tabs, the
checkpoint ribbon, the tutorial pop-ups, the level-complete screen (ScoreTallyScreen), the pause menu.
(20) End of a level: `EventManager` (§_-7t§: loadEvents / addEvent wire each room's eventArray), triggers between
objects and enemies (`GameObject.sendTriggered` / `broadcastTriggered`, `ObjectManager` / `EnemyManager`
activateObject, setEvent, setEventTriggers; the camera pan is SKELETON: objects react at once), riding objects
(`CustomerChar.checkForGrabbingTile`, rideable part), `GameObject1` cage pulley, `GameObject19` / `GameObject20`
cage (GameObject20's constructor read from the P-code), `CageFrontBars` / `CageBackBars` / `CageSpinner` effects,
`PlayerChar.finishLevel` and the finish sequence (walk, four jumps, end portal, iris, fly in, white flash),
`GameDisplay.startMainIrisOut` (the `deadIrisOut` clip, drawn unmasked) and its end (`§_-4R§.finishLevel`,
`§_-2K§`), `§_-9k§` (fresh player and managers per level; level changes run at the start of the next logic frame,
`runPending`), `UserData` customer unlocks (`userdata.json`), `CustomerData.getTrappedCustomerIndex`.
DEV `--at x,y` puts the player on a tile (`--room 2 --at 12,33` in level 0 drops him on the pulley).
SKELETON: the level-complete and game-over screens (the next level / the same level starts), CageCustomerEffect
(the trapped customer's puppet; only Marty and Rita are in the customer list), the boss "fake" finish, grab tiles,
the camera pan, checkpoints restoring a level (CheckpointData), masks (the iris).
Note: flipped frames are the TOP half of each sheet, unflipped below (`flipBlitOffset`).
Steps were: (1) importer writes `framescripts.txt` from the ABC; (2) `engine::MovieClip`: named children, attached
children, visibility, `gotoAndStop` running frame-script data; (3) `CharacterBlitter` into render targets with
the outline; (4) `CustomerData`/`CustomerDataFile`/`CustomerMarty`; (5) `reblitMultispritePlayer` + standing
player at the room's `startPoint`; then `Character`/`PlayerChar`/`CustomerChar` movement and collisions,
`GameControls` (actions: keyboard, gamepad, touch), `GameCam.adjustCamera`.

## Later

Enemies, objects, items, HUD/menus (needs masks/filters/text → SDL_GPU renderer), widescreen, options
menu, translations, touch-control editor, PL3 (shares the engine), PL1 (AS2, separate).
