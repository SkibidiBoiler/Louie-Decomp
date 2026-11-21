# Progress and next steps

Read this first when resuming. Rules and commands: [AGENTS.md](../AGENTS.md).
Map of the original code (systems, enemies and objects per world, order for what is left):
[architecture.md](architecture.md).

Translation progress, counted locally: `python -I tools/progress.py` (functions and lines of the game's AS
whose `// AS:` tag exists in `src/game`; SKELETON code counts, so it is an upper bound). On 2026-11-16, PL2:
697 of 2263 functions (30.8 %), 13.1K of 37.7K lines (34.8 %), 105 of 232 classes started.

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
checkpoint ribbon, the tutorial pop-ups, the pause menu.
(26) Level complete (menus, part 4). `§_-4R§.finishLevel` translated whole (time played; the train loop is
SKELETON) -> `§_-4G§` `StageOutroScreen` (the customer and the rescued one fall through the warp with the coins
and warp coins earned) -> the API's EndOfDay, `ScoreTallyScreen`: the values are counted up one after another
(points, time bonus, challenge bonus, level score; high score and total only when it is a new high score; coins),
the level's challenges with the rescued faces, the player and the rescued customer jump in among three unlocked
customers who cheer. Getting the values records the level in the slot (unlockNextLevel, setLevelTime,
updateAndGetLevelScore, which saves). Bar: LEVEL COMPLETE, QUIT and Continue; the iris closes, then `§_-Ap§` tears
the level down (the port keeps BitmapManager: it holds the sheet textures) and the map opens (Quit: the title).
`§_-6V§` now resets lives and tallies and updates the HUD. Engine: code-set rotation (`MovieClip::setRotation`),
`Lang::get(key, value)` for "{0}" texts.
Checked: `--level 0 --room 2 --at 12,33` -> cage -> outro -> tally (capture) -> tap Continue -> map; the slot
saved the high score and time.
SKELETON: the parade at level 8 (§_-14§ shows the tally), BossIntroScreen,
ChallengeManager.resetCurrentChallenges, the glow outline of the puppets.
(27) New customer and pause (menus, part 5). `NewCustomerScreen` (§_-4Z§ / §_-6s§): a customer rescued for the
first time runs in with their name and skill, then the map. Pause: the HUD's MENU and sound buttons
(`GameHUD.clickMenu` / `clickMute` / `clickUnmute` / `updateMuteButton`; the HUD takes the pointer during a level),
`GameControls.pauseGame` / `resumeGame`, the API's PauseMenu (Back to Game, INFO, CREDITS, QUIT LEVEL) on
`MainMenuScreen`: `setupInfo` (the area, the customer being played, the challenges with their tallies,
`ChallengeManager.getChallengeTallyString`, this level's and the game's totals, `buildInfoModel`),
`setupConfirmQuit` (YES / NO; the bars stop taking clicks meanwhile, `MenuBar::setEnabled`), `clickBackToGame`,
`clickQuit` / `clickConfirmQuit` / `clickCancelQuit`, and `closeMainMenuScreen` quitting the level (time played,
save, §_-Ap§, the map) or resuming. Port: P opens the pause menu too. Engine fix: `mouseEnabled = false` no longer
hides a clip's children from the pointer (Flash: that is `mouseChildren`).
Checked by taps: MENU -> pause (capture) -> Back to Game -> MENU again; QUIT LEVEL -> confirm (capture) -> YES ->
map. Level 0 cage -> tally -> Continue -> "Customer unlocked!" Taylor (capture) -> map.
SKELETON: the BADDIES, BADGES, CONTROLS and HELP tabs.
(28) Challenges (task 15, part 1). `ChallengeManager`: recordTag / unrecordTag, recordCustomerCage,
recordBurgerzilla, recordCoin, recordSpecialItem (QUIRK: these three add one whatever the amount),
recordMultiExplosion / Rocket / Juggle, recordCombo, recordCompletionTime, recordEnemyKill, recordMastery,
checkCustomChallenges, the checkpoint tallies, resetCurrentChallenges (each level starts from 0), the completion
and badge queues; badges are earned with their reward. `Challenge`: every type's fields (read from the importer's
params), the tally methods, meetsEnemyRequirements, meetsMasteryRequirements. Hooked: coins (and Georgito's tag),
cages (and Xandra's in level 9), stuns, cracker blocks, Marty's and Greg's tags, combos, the finish time and the
custom challenges at the end of a level.
Checked: level 0 cage -> "COMPLETE CHALLENGE #2", the tally shows it gold with its 500 bonus and one warp key.
SKELETON: the drop-down that announces challenges and badges (checkForDisplayingDropdown), juggles
(meetsJuggleRequirements), ObjectManager.getWeaponType for mastery.
(29) Game over. `§_-4R§.§_-2K§` (out of lives: time played, save, §_-Ap§, the map; PL2 has no game over screen)
and `§_-99§` (died riding an elevator: the level again), chosen by `GameDisplay` when its death iris ends. The
display calls them from its own update, so their bodies run at the start of the next frame (`runPending`, now
before the level's tick). Checked with a forced death (temporary DEV code, removed): DEAD -> saved -> map.
(30) Bullets, part 1 (task 6). Importer: `bullets.json` (the field constants of Bullet and the 19 bullet
classes). `Bullet` (extends Character): hitting enemies, the player and objects, bouncing, the direction's row
block, dead zones. `BulletManager` (§_-4l§): add / process / clear / remove, one sheet per type, addToPreblit and
preblitSheets (from DataManager.initializeScreenObjects). `CustomerChar.hasProjectile` / `shootWeapon` (the muzzle
offsets from customerchar.json; Xandra's three stars), fired from checkIfAttacking. `GameDisplay.reblitBullets`
(behind and in front). `Basketball` (Connor): the arc, two bounces, the burst that hits around it.
`Character.checkIfOnSlope`; `checkIfOnCloud` is virtual (Basketball overrides it). DEV: `--trace` prints a B line
with each bullet's id, type, position and frame.
Checked: `--customer 16 --input A100-104` -> the ball arcs right and bounces (trace and capture).
SKELETON: the bazooka's camera jiggle.
(31) Bullets, part 2: `DogToy` (Prudence: squeaks, three bounces), `PepperBomb` (Big Pauly: explodes where it
lands; `ExplosionEffect`), `GummyOnion` (Sarge Fan: four bounces, higher then lower), `Cookie` (Yippy: crumbles
where it lands; `CookieBitEffect`). Checked by trace with `--customer 2 / 8 / 13 / 19`.
`RomanCandle` (Boomer: a curving rocket with a smoke trail that stuns; `RomanSparkEffect`, `RomanTrailEffect`).
`XandraStar` (three stars in a fan, twinkling; `StarEffect`).
(32) Bullets, part 3: `Banana` (Akari: curves until it hits a wall or the floor, then bounces and bursts on the
second bounce; `SmokeRingEffect` following the player, `CheeseBitEffect`), `ChiliCan` (Rico: lobbed, bursts
with `ChiliBitEffect`, `BBQBitEffect`, `ChiliSplatEffect`), `BlueberryShot` (Penny: straight, then curves down;
`BlueberryBitEffect`, `BlueberrySplatEffect`). Their `addSplatter` stains are `GameDisplay::stampOnTiles` (the
port has no pre-rendered tileBMP: stamps in level coordinates, drawn after the tiles, cleared by setupPreBlit).
Checked by trace with `--customer 6 / 22 / 12` and a capture of the chili stain.
Note: `--trace` must not be the last argument (the option loop reads pairs).
SKELETON: the enemies' and the cheese cannon's bullets.
(33) Lettuce Lane: `Enemy7` (Roma tomato: hops forward non-stop, turning at walls), `Enemy12` (slider: winds up,
slides and slows, turns; a stomp hurts it), `Enemy14` (lark: flies between two rows; a hit knocks it down in
`FeatherEffect`s, then it hops). Standing on objects: `GameObject.checkStartStepping / stepOnObject /
stopStandingOnObject / checkStandingOnObject`, `ObjectManager.checkSteppingOnObjects` (called by the player),
`Character.checkMovingTiles` and `whichMovingTile`, the tile's `standingWalkSpeed`, and the draw order (the
ridden object last; the player over a foreground one). Port: `ObjectManager::releaseStandingChars` clears
`whichMovingTile` when its object is freed. Blimps `GameObject4` / `GameObject5` (bob, sag under the rider,
tween in for bosses). The spawner enemies' `removeEnemy` now calls `unregisterSpawn` (Enemy1, 2, 6).
DEV: the trace's `O` line lists the steppable objects (`r`: ridden). Checked by trace (level 2 room 2-3,
level 3 room 2) and a capture of the player on a blimp.
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
SKELETON: the level-complete and game-over screens (the next level / the same level starts),
the boss "fake" finish, grab tiles,
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
(34) Fort Onion, part 1 (work in progress): `Enemy3` army onion (collapses into its helmet when stomped), `Enemy4`
gold army onion (drops `GameItem_GoldHelmet`, `ItemGoldHelmetEffect`), `Enemy8` runt tomato, `Enemy15` leafy lark,
`Enemy16` dill wheel (its own `moveChar` round floors, walls and ceilings; `obf_8J` linear easing). `moveChar` and
`adjustSpeedForSlopes` are virtual; EnemyManager gives subclasses their parent's cycle arrays. Fort Onion is
`--level 2`. Checked by trace: the army onions (room 1), the runt tomatoes hopping (room 2, `--at 76,14`), the leafy
larks flying their rows (room 3, `--at 10,6`), the dill wheels rolling round floors, walls and ceilings and hurting
the player (room 4, `--at 60,21`); the gold onion of room 1 (`--at 59,41 --input "J38-46,L60-70,R71-72,A73-75,A85-87,
R100-140"`) drops its helmet (goldhelmet_drop sounds), which is picked up (specialitems 1, ItemGoldHelmetEffect).
(35) Fort Onion, part 2: `GameObject6` / `GameObject7` horizontal blimps (shuttle between their column and a target
column, or wait for the player; carry their riders; GameObject6 can be a boss's, tweening in), `GameObject17` floor
switch (pressed by stepping on it; objects too), `GameObject18` door (solid while shut, four tiles; sorts itself
and its front frame against the player) and `DoorFrontEffect`. `GameObject.stepOnObject` is virtual. The blimps'
and the constructors' debug trace() calls are left out. Checked by trace: room 0's switch opens its three doors
(papow ×3) and the player walks through (without it he stops at the first door); room 3's blimps 5 (a waiter) and
10 (`--at 80,13`, small) carry the player; blimp 8 shuttles on its own. Captures of the open doors and the small
blimp. Data for tests: `pl_import` output in `datos-papas/build-cur` (`--data` that folder) (outside the repo).
(36) Fort Onion, part 3: `GameObject33` training sensor (on overlap or on screen it opens a bubble; off screen
again, a basic training counts as done), `TrainingPopup` (trainingMC at the flag's frame, key caps, slides in and
out, timeout), `§_-4R§.§_-6n§ / §_-c§ / §_-64§` (open / done / close the bubble, `§_-a§`), the `§_-9X§` container,
`DataManager.setupKeyNames / getKeyLabel`; `CageCustomerEffect` (the trapped customer struggles, stands up when the
cage opens, jumps with the player, flies into the portal) and `BitmapManager.createCagedSprites / finishedCageBlit`
(the caged sheets, blitted after the player's in the stage intro, and in the `--level` start). `Effect.getSheet` is
virtual. Port: a TextField's `visible = false` is an empty text. Checked by captures (level 0: "Use <- -> to Walk",
then the jump bubble with the Up key; the struggling customer in level 2 room 4 and level 0 room 2) and trace (level
0 room 2 `--at 12,33`: the customer jumps with the player and is pulled into the portal).
The other training calls: `PlayerChar.startupPlayer` (the skill bubble: groundpound / glide / doublejump),
`startPounding` / `startDoubleJumping` / `startGliding` / `startWallJumping` and crawling (done), the MainMenuScreen
bubbles (character, challenges, styles, nowarpkeys; `showSectionTraining` is the part of tweenSections that opens
them) and `ObjectManager.updateTrainingSigns` (`GameObject.updateTrainingSign` is empty). Checked by captures
(`--level 2 --customer 8 / 23 / 15`: the pound, glide and double jump bubbles; the double jump closes its bubble).
Not checked: the menu bubbles.
(37) Sarge's Lair (`--level 3`), part 1: `GameObject14` drop berry (stood on, it drops after 20 frames with its
riders, splats and grows back), `GameObject2` pushable boulder (pushed by the push skill, rolls, slows, rolls down
slopes, carries riders, squashes enemies; its "push" training), `GameObject.checkPushingObject` / `isBeingPushed` /
`saveDataAtCheckpoint`; `Enemy5` Kaiser onion, `Enemy9` / `Enemy10` / `Enemy11` Party Sub (a group walks together
and waits while one car is stunned: `EnemyManager.sendMessageToEnemyGroup / getEnemyIDsFromGroup /
checkGroupOnScreen`), `Enemy17` Dill Worm (hangs, notices the player below, drops, crawls after the player).
Checked by trace: room 0 berries (`--at 5,21`) and boulder (`--at 96,17 --customer 14 --input "R20-120"`; Marty
cannot push it), the Kaiser onion (`--at 52,13`), room 1 Party Subs (stomping one car of the group at
`--at 29,24 --input "L60-75,J66-74"` stops the others until it recovers) and room 2's Dill Worm
(`--at 78,56 --input "L20-45"`).
(38) Sarge's Lair, part 2: the boss. `Enemy35` Sarge (dropped into the arena by the "Start Fight" trigger; jumps
about and throws `SargePepper` (style by distance: flat, normal, high) or, once hurt, alternately `GiantPepper`; a
stomp only stuns him, a ground pound (or a bullet, a quarter point) hurts him; hurt, he rolls for 200 frames, stops
at x tile 20 and calls static blimps (more at one point left), which he jumps onto; at one point left he calls in
six Kaiser onions, which die with him), `GameObject29` "Start Fight" (closes the doors and drops the boss; can lock
the camera), `GameObject30` "End Fight" (unlocks the camera, opens the doors; with "spikes" it first plays the
escape scene: `LaughEffect`, `BossEscapeEffect`; used in a later world). `GameCam.setCameraJiggle / setCameraLock /
unlockCamera / jiggleCamera`, now also called by the player's ground pound, hurt, death and bazooka and by objects
with `jigglecam`. `Enemy.sendTriggered` is virtual. DEV: the environment variable `PL_SEED` fixes `Math.random`'s
seed so traced runs repeat. Checked by trace (`--level 3 --room 4 --at 2,25`): the drop, landing, throws and jumps;
with `PL_SEED=3 --customer 8 --input "R20-60,R102-124,J102-112,D124-127,L130-150"` the stomp stuns him, the ground
pound hurts him, he rolls, stops at tile 20, three blimps appear and he jumps onto them and throws giant peppers.
With his health set to 1 for a local test (not committed), a ground pound kills him: the six onions die with him,
"End Fight" switches off and the doors (closed by "Start Fight") open again.
`BossIntroScreen` is not Sarge's: it shows `boss_intro_radish` (the radish boss) over tileset 4's backdrop, so it
goes with that world. Not checked: the camera lock (this arena does not use it) and the escape scene (a later world).
(39) Mount Monterey (`--level 4`): `GameObject8` / `GameObject9` vertical blimps (shuttle between two rows, or wait
for the player), `GameObject12` / `GameObject13` dropping blimps (sink to a target row while ridden, rise back when
left); `Enemy13` Brussel Lark (flies round a circle; a stomp kills it), `Enemy19` Blue Shroom (ducks and rises on a
timer, faces the player, cannot be hit while down), `Enemy25` Swiss Zack (a crusher that cannot be hurt; its hit box
follows the crusher), `Enemy26` Cheddar Mack (swings on a rope from the ceiling as a damped pendulum; the rope is
drawn with the new `GameDisplay.drawScreenLine`, standing in for `screenBMP.draw` of a line; hits push the swing),
`Enemy32` Jellyback (harmless, cannot be killed; stomping it throws the player high). Checked by trace with
`PL_SEED=3`: room 0 larks circle at their radius (`--at 80,12`), the vertical blimp carries the player up to row 16
and back (`--at 54,22`), the small dropping blimp sinks to row 19 and rises after the player steps off (`--at 110,10
--input "R100-112"`); room 1 the Cheddar Mack swings (`--at 18,34`, capture shows the rope); room 2 the Blue Shroom
cycles down 20 / up 45 frames (`--at 24,10`), a Jellyback bounces the player about 260 px (`--at 24,46 --input
"R30-40,J30-40"`); room 3 the Swiss Zack cycles, thumps and hurts the player (`--at 20,40`, capture). Not checked:
killing a lark or a Cheddar Mack, hitting a Blue Shroom.

(40) Cheddar Hills (`--level 5`): `Enemy18` Dill Weed (rooted; chomps forward, left and right in turn with a hit box
that grows with the chomp), `Enemy21` Bow Shroom (the Blue Shroom's duck cycle plus `ShroomArrow` shots, optionally
only when the player is in sight), `Enemy27` Pepper Jack (notices the player within 160 x 96 px, charges faster and
faster, skids 64 px past and turns; at a ledge it jumps towards the player instead of turning, so
`Character.checkIfAtEdge` is now virtual), `Enemy31` Cheese Wheel (rolls one way, a stomp beats it);
`GameObject10` / `GameObject11` circular blimps (fly round a circle, optional rope to the centre drawn with
`drawScreenLine`), `GameObject26` Cheese Cannon (fires `CheeseBullet` at the player every shootDelay frames). The
cheese: `PlayerChar.getCheesed` / `removeCheese` / `jumpCheesed` and `StuckCheeseEffect` (three jumps shake it
off), plus `CheeseSplatEffect`; the `isCheesed` SKELETON gaps in `GameControls` and `CustomerChar` are filled.
Checked by trace: room 0 a cannon shot cheeses the player (`--at 11,17`), walking does nothing and the third jump
frees them; Dill Weeds cycle their chomps with sounds (`--at 22,33`); room 1 a Pepper Jack charges, hits, skids and
turns (`--at 61,40`); room 2 four circular blimps circle and carry the player; room 3 a Bow Shroom's arrow hurts the
player (`--at 29,87`). Not checked: the Cheese Wheel (spawned by objects off screen), a tethered blimp's rope (no
tethered one in Cheddar Hills). Found and fixed in the importer (below): the blimp sheets showed the balloon only
on frames 3, 10 and 13.

(41) Importer (`src/import/raster.cpp`, `pl_import`): morph shapes (`DefineMorphShape` / `DefineMorphShape2`) are
now drawn, as masks too, interpolated at the placement's ratio; before, they were skipped, so a morph used as a
mask hid everything under it (the balloon of every `object_blimp_*` sheet is masked by a morph circle). Every
vector sheet now starts at a whole stage pixel (the index origin is floored): the tilesheet parts had fractional
origins (e.g. -24.67, -19.03), were pasted into the composite with resampling, and left faint seams at the tile
edges (level 1's dirt showed a grid). `tools/swf2assets.py` gets the matching morph workaround for FFDec (morphs
filled only on side 0 are refilled on side 1 in a working copy, since FFDec 26 renders them inverted). Checked by
capture: level 0 room 1 without seams, level 5 room 2 with the circular blimps drawn. Data regenerated with
`pl_import` into `datos-papas/build-cur` (the previous data kept in `build-cur-old-20261120`).

(42) Ladders, pick-ups, options. `GameControls.jumpPlayerOnLadder`: pressing left or right on a ladder hops off it
(the original calls it from keyDownListener; the port on the press of the action), so the player can jump from one
ladder to the next (level 1, room id 27: up the ladder at column 10, right, up again on the ladder at column 14;
checked by trace). Item pick-up effects are added at (0, 0) and follow the player from their first update; their
interpolation now starts on the player, which removes a one-frame flash in the screen's top-left corner. The port's
options menu (`src/engine/options_menu.*`, not in the original): a translucent gear button in the window's top-right
corner (or F1) opens a translucent panel, in the touch buttons' style, with the frame rate limit, smooth movement,
language, button transparency and size, and the FPS line; saved to settings.ini at once; the game pauses while it is
open (DEV `--options` starts with it open). The shown display rate follows Android's mode switch (the window asks
for the panel's fastest mode, and the POCO does switch to 120 Hz with the app in front).

(43) The Saucelands (`--level 6`): `Enemy20` Thorn Shroom (the Blue Shroom's duck cycle, cannot be stunned: its
thorns hurt a stomping player), `Enemy22` Bacobar (a tall bacon wall walking between two columns: hits make it
wobble and a player inside it is pushed out 16 px and hurt; stomped on its top it crunches to half height and runs
faster, and only then can be beaten), `Enemy28` Radish (walks after the player; a planted one has no hit box until
the player is near, then pops out throwing `RadishDirtEffect` clods), `Enemy29` Laddish (climbs up and down a
ladder; knocked off, it climbs again once recovered); `GameObject27` BBQ quicksauce (a tiling pool: the player sinks
1 px a frame and walks at the quicksand speed 5; enemies in it are swallowed) and `GameDisplay.blitTilingObject`
(the tiling objects were drawn as plain sprites). `climbDir` moves from CustomerChar to Character, as in the original.
Checked by trace: room 0 the pool (`--at 123,17`, capture), room 2 a Bacobar pushes and hurts the player
(`--at 124,35`, capture), the planted radishes pop out (`--at 118,5`), room 3 the Laddish climbs and turns at the
ladder's end (`--at 15,15`) and the Thorn Shrooms turn and duck (`--at 22,66`). Not checked: crunching a Bacobar,
knocking a Laddish off its ladder.

(44) BBQ Bog (`--level 7`, rooms 8-A to 8-D, ids 55–57 and 60): `GameObject15/16` On and Off Berries (platforms that pop up
and go down on a timer, wobbling for the last 20 frames; nothing stands on one while it is down), `Enemy23` Bacoburn
(the Bacobar's wall, but a stomp, slide or ground pound beats it at once and breaks it into three `Enemy24` Baco
Bites, which hop back and forth), `Enemy30` Pararadish (floats down 2 px a frame on a parachute, folds it on landing,
then hops after the player), `Enemy33` Awesome Saucer (glides between two columns on a sine wave, slowing into each
turn, and drops a `SauceBullet` on a timer, with `SauceBitEffect` / `SauceSplatEffect` where it lands). The
Pararadish's spawner is an enemy (the Radley Madish boss, not ported yet): `Enemy.unregisterSpawn(type, id)` is a
port virtual with an empty body until Enemy37 overrides it. Checked by trace: room 0 a berry carries the player and
drops them after its 60 frames (`--at 27,80`), room 1 a saucer turns at its target column and its sauce splats
(`--at 36,17`), room 2 a ground pound (`--customer 8 --at 20,60 --input D3-5`) crunches a Bacoburn into three hopping
bites (capture), and a player inside a Bacoburn is pushed out and hurt; room 3 the Pararadishes land, fold their
parachutes and hop at the player (`--at 25,12`, capture). Note: falling onto a Bacoburn at full speed usually ends in
the push-out rather than a stomp (its own update pushes the player out in the frame they reach its body, before the
player's next stomp test); not compared with the original. Progress: 72.8 % of functions, 79.9 % of lines.

(45) The `--level` index is the world's tileset, not its place in levels.xml: 7 BBQ Bog (item 44 first called it
Radley's Hideout by mistake), 8 Radley's Hideout (rooms 53, 59, 58, 17), 9 X Zone; `tools/pl_check.py worlds` lists
them. Radley's Hideout room 9-B (`--level 8 --room 2`): `Enemy36` the first boss (sheet "easyboss": once the Start
Fight trigger fires he runs back and forth, blocks every hit unless stunned, throws a `Beaker` every 120 frames;
beaten, the music stops with the fanfare), `Beaker` (generated from PepperBomb: breaks into `BeakerGlassEffect`
shards and three `BeakerBlob`s, generated from Cookie, which puff into dust) and `GameObject31` Spike Bed (kills the
player or any enemy that touches it; once triggered it shakes, then rises 2 px a frame to its target row). X Zone:
`GameObject32` Rainbow Sauce (the quicksauce pool with its own sheet, a bottom cap and `RainbowBitEffect` splashes).
Checked with `pl_check.py run`: walking left into the trigger (`--at 5,79 --input L20-34`) starts the boss, who runs,
turns and throws beakers that break (capture); the spike bed kills the player (`--at 6,86`); in X Zone room 1 the
player sinks into the rainbow pool and splashes (`--at 12,30`). Not checked: stunning and beating Enemy36, the
spike bed rising. Left in Radley's Hideout: `Enemy37` Radley Madish with `RocketBBQ` / `RocketMustard` (room 9-C).
Progress: 75.2 % of functions, 82.4 % of lines.

(46) Radley Madish (`Enemy37`, Radley's Hideout room 9-C, `--level 8 --room 0`): triggered, he flies after the
player (easing to 180 px beside and above them, on a wave) with `BossPropellerEffect` and the looping copter sound,
and on a timer grabs, loads and fires a rocket: `Enemy38` BBQ rocket / `Enemy39` mustard rocket, enemies that home in
on the player turning at most 0.06 rad a frame (`setAnimationAngle` picks the frame of the angle rounded to 10
degrees) and splash on a wall. A hit knocks a BBQ rocket away; a hit mustard rocket turns round and homes in on him,
and it knocks him down: he lands, runs for 120 frames (stun him, then hit him), then takes off again. With 3 hit
points he lets out one Pararadish at a time, with 2 up to three, with 1 a Burgerzilla and BBQ rockets in bursts of
three; beaten, the music returns 150 frames after the fanfare (`CustomerChar.bossDelayMusicTimer`, the PlayerChar
fields). The Pararadish and Burgerzilla now call his `unregisterSpawn(type, id)`; `EnemyManager.enemyByID` is the
port's `manager["Enemy" + id]`; `Enemy.placeEnemy` is virtual (the rockets are placed in pixels). The bullets
RocketBBQ / RocketMustard are not added by any code and stay untranslated. Checked with `pl_check.py run`: walking into
the trigger (`--at 6,16 --input L20-34`) starts him; he flies, fires BBQ rockets that splash, lets out a Pararadish
(capture); attacking every 8 frames after stepping right (`R40-60` plus `A` pulses) hits a mustard rocket back and he
falls, lands and runs. The propeller is drawn (green, behind him; it is easy to miss against the trees). Not
checked: stunning and hitting him on the ground, his 2 and 1 hit-point phases, the end of the fight.
Progress: 77.4 % of functions, 84.7 % of lines.

(47) Checkpoints: `CheckpointData` (a copy of every room's data with the starting room's start point moved to the
checkpoint tile, plus the hurt / water flags, kills, lives lost, gameplay time, money, Burgerzillas, special items
and the counted challenges' tallies). `DataManager.saveCheckpoint` / `clearCheckpoint` keep it,
`setupLevelScreens` loads the level from it when it is for that level ("Use Checkpoint Data"), and
`handleCheckpointProgress` (now called when a level starts) restores the tallies. `ObjectManager.saveObjectsAtCheckpoint`
(the objects with saveDataAtCheckpoint: the boulders) and `ItemManager.saveItemsAtCheckpoint` write the room's live
state first. The map shows the checkpoint ribbon on that level. Dying with lives left still respawns at the last safe
spot (`restartPlayer`), as in the original; the checkpoint is used when the level is entered again.
`tools/pl_check.py run --grep RE` prints matching log lines. Checked: touching the checkpoint in Leaf Landing room 1
(`--level 0 --room 1 --at 15,37 --input R10-30`) saves it ("4 room states saved"); DEV `--reenter N` starts the current level again at
frame N (as `--level` does): with `--input R10-30 --reenter 90` the level loads from the checkpoint ("Use Checkpoint
Data", tallies restored) and the player starts in room 1 at tile 17,38. Pause menu, quit and its confirmation work
with `--tap "35,398@60;80,20@100;274,232@160"`, but a game started with `--level` has no slot loaded, so the map's
world buttons do not answer: the ribbon on the map is not checked.

(48) Effects: `FoodiniBalloonEffect` (Foodini's balloon on a string, at most 60 px from his hand: it rises, is
pulled along, bobs on eased quarter waves, leans one frame per 10 degrees, hides during his attack, floats away when
he is gone), `FoodiniGlideEffect` (the six small balloons of his glide: they ease to their spots above him and,
when the glide ends, float away, or are reeled back in if balloons were let go less than 40 frames before),
`WarpCoinEffect` (the warp key rising over the player) and `ItemGlowEffect` (its glow). The strings are drawn with
`drawBalloonString` (straight, or the original's quadratic curve as short lines). CustomerChar's Foodini SKELETONs
are filled: the balloon at startupPlayer, hidden on attack, reset at the attack's end and in cancelAttack
(`resetFoodiniBalloon`, a port helper), the glide balloons (QUIRK kept: a balloon already reeled in throws in the
original, which leaves the list uncleared). Checked with `--customer 25` (Foodini): the balloon follows him (capture)
and the glide balloons appear and rise away. Not checked: the warp key effect (end of a level), the strings'
look. Progress: 79.2 % of functions, 86.3 % of lines.

(49) `BossIntroScreen`: on the first play of Radley's Hideout (level 8, high score 0) the level start shows Papa
Louie trapped (customerTrappedMC) facing the radish boss over the tilesheet 4 backdrop, with MidBossTrack; after 120
frames the iris closes and the level starts (obf_4R obf_6I / obf_28; the SKELETON before obf_52 is gone). QUIRK kept:
the puppet holds the selected character's weapon. The GlowFilter on the puppets is not drawn (no filters in the
port). Engine: `MovieClip::addChildAtBottom` (addChildAt 0, under the timeline children). `CustomerData.getCustomerIndex`.
`PlayerStunSparkleEffect` translated and registered (PL2 never adds it; build checked only). Checked: capture of
the intro at frame 70 (`--level 8`) and the trace of the level starting after it. Progress: 79.9 % of functions,
87.1 % of lines.

(50) `ParadeScreen` and `GameOutroScreen`: after the last boss (StageOutroScreen of level 8) every unlocked customer
marches across in roster order (running puppets with weapons; the next one starts when the newest is 200 px in, or
80 px when they share a skill), then an iris opens the ending: the unlocked customers posed in three rows around Papa
Louie under "The End"; after 250 frames the EndOfDay tally. obf_4R obf_14 (the SKELETON that showed the tally is
gone), obf_3q, obf_49, obf_2r. QUIRK kept: releaseNextGroup's group size is an unset local, so marchers are never
staggered. DEV `--parade N` shows it at app frame N. Checked: captures of the parade (Marty and Rita) and of the
ending. Not checked: the walking/waving parade (showWeapons false, only from the API menu), the hand-over to the
tally. Progress: 80.9 % of functions, 88.9 % of lines.

(51) Challenge drop-down: ChallengeManager checkForDisplayingDropdown, animateBadgeDropdown, clickBadgeDropdown
(the queued treasure, challenge or badge eases down from the top, stays 150 frames or until clicked, goes up, and the
next follows; QUIRK kept: a treasure's panel gets neither part set up), getChallengeTitle / Description /
RewardAmount, getBadgeTally / TallyString, shouldLockBadge (Challenge gets requiredCustomers / requiredLevel and
getRewardAmount). The original adds the drop-down to the game object itself: the port's `obf_4R::rootClips` holds it
above the screens and takes the pointer when it is under it. Checked: rescuing Prudence in Leaf Landing shows
"Challenge Complete! Rescued Prudence!" (capture) with getstar.wav. Not checked: a badge's panel. Progress: 81.4 % of
functions, 89.3 % of lines.

(52) `tools/progress.py` leaves out what the port drops or replaces on purpose (EXCLUDED: online services, sponsor
and loading screens, Flash bitmap drawing and asset loading, the never-created RocketBBQ / RocketMustard): 126
functions, 1423 lines; `--excluded` lists them with the reason. Progress over the ported scope: 86.1 % of functions,
92.8 % of lines.

(53) Menu BADGES tab: MainMenuScreen setupMedals, populateMedals, clickNextMedals, clickPrevMedals,
animateMedalsTransition, clickMedals; the tab is on the main and pause menu bars ("clickMedals"). Eight badges a
page with title, description and tally, reward, points, picture and earned mark; locked ones show "Hidden Badge"
(port texts badge.* in lang/). Not drawn: the grey (ColorMatrixFilter) on unearned badges. The drop-down's reward and
points texts use the same keys. Checked: captures of pages 1 and 2 from the pause menu. Progress: 86.4 % of functions,
93.1 % of lines.

(54) Menu BADDIES tab: MainMenuScreen setupBaddies, clickEnemyThumb, selectEnemy, clickBaddies, the roll-over /
roll-out of the thumbnails; DataManager enemyKey (from datamanager.json), getEnemyName, getEnemyClipName,
getEnemyIDFromIndex. The thumbnails are the enemythumb_<clip> clips themselves (the original draws them into one
bitmap with a drop shadow: generateEnemyBitmap / getEnemyBitmap, now in progress.py's EXCLUDED). Port texts
baddies.* in lang/. The tab is on the main and pause menu bars. Checked: after killing three Red Tomatoes in Leaf
Landing the tab shows 1/33, the tomato's thumbnail, picture and "Derrotados: 3" (capture). Progress: 86.8 % of
functions, 93.6 % of lines.

(55) Menu CONTROLS tab: MainMenuScreen setupControls, clickControlsButton, cancelSettingControls (also when leaving
the section), controlsKeyListener, the buttons' roll-over / roll-out, clickControls; the tab is on the main and pause
menu bars. The keyboard now follows the player's keys: engine `Input::keyBindings` (Flash key codes, set from UserData
every frame) with `flashKeyToScancode` / `scancodeToFlashKey`; the port's extra keys (W A S D, Z, X) work while no game
key uses them. While the tab waits for a key, the app sends the next key to `obf_4R::keyDown` (the original's stage
KEY_DOWN listener). Checked: capture of the tab with the six keys. Not checked: changing a key (no scripted raw keys).
The key labels' missing letters are fixed (see 57). Progress: 87.1 % of functions, 93.9 % of lines.

(56) Menu HELP tab: MainMenuScreen setupHelp, setupHelpKeys (also when the section opens), showHelp, clickHelpTab,
the tabs' roll-over / roll-out, the two scroll bars (click / hold / release of the arrows, click / drag / release of
the dragger, for the tab list and the page), clickHelp. The 18 topic names are port texts (help.topic1..18 in lang/).
Port: drags keep the grab offset (startDrag), the stage MOUSE_UP is the pointer going up, `obf_4R::pointer` holds the
frame's pointer; moving a timeline child vertically keeps its timeline x (setChildY). QUIRK kept:
releaseHelpMainArrow clears helpTabScrollDir. Not ported: the `useSection: "help"` parameter (never sent). Checked:
captures of topics 1 and 2 (Attacking shows the player's Space key). Not checked: the scroll bars. The pause bar's six tabs fit (see 57). Progress: 88.0 % of functions, 94.5 % of lines.

(57) Text and menu bar fixes: a character missing from a field's embedded font is drawn from the embedding of the
same font with the most glyphs (Flash embeds only the characters a field was made with; the CONTROLS labels showed
"Sace", "Let"). The menu bar's tab row starts after the back button when it would go under it, with narrower buttons
(no side padding) when it would run off the right edge (the pause menu's six tabs). Checked: captures of the CONTROLS
tab (Space, Up, Left, Right) and of the pause bar.

(58) Fix: the sky and backdrop of a level played after another kept the first level's. The original makes a new
BitmapManager for each level (its bgsheet / mgsheet start null); the port keeps one, and getTilesheetBitmap sets
whichTileset before getBackgroundBitmap / getMiddlegroundBitmap ask, so they returned the old sheets. obf_6V now drops
them. Note: worlds share backdrops in the original (bgNames / mgNames): Leaf Landing and Lettuce Lane use tilesheet1,
Fort Onion and Sarge's Lair tilesheet2, Mount Monterey and Cheddar Hills tilesheet3, The Saucelands, BBQ Bog and
Radley's Hideout tilesheet4, X Zone tilesheet5. The room order per world is levels.xml's roomArray (Radley's Hideout
starts in room 2, "9-A"; Sarge's Lair's 4-F is its fourth room), as in the original. Checked by reading the code
path; not checked by playing two levels in a row.

(59) Engine fixes from the owner's phone captures: nested masks now clip (MaskLayers keeps one texture pair per
nesting depth and composites onto the outer one; before, a mask inside masked content was skipped and its hidden art
showed: the second portal ring in the character select's level circle). The menu bar's bottom panel hides on the
CREDITS section when it holds no button (the credits picture reaches the bottom of the stage). Checked: captures of
the map (unchanged) and of the credits (whole picture). Not checked: the character select (needs a save with a level
unlocked) and the map's black cloud seen on the phone.
