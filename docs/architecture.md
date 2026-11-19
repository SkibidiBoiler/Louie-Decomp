# Papa Louie 2: map of the original code

What each part of PL2's ActionScript does, how the parts depend on each other, and the order to translate what is
left. It describes the original code; it does not contain any of it. Read it before starting a new system, then
read the functions themselves with `tools/as_func.py`.

Sizes are non-blank lines of the decompiled class. "Done" is the share of a group's lines in functions that already
have their `// AS:` tag in `src/game` (SKELETON code counts). `python -I tools/progress.py --by-class` gives the live
figures; the ones here are from 2026-11-19.

## The frame

```
obf_4R  (§_-9G§.§_-4R§, the game's root clip; owns everything below)
  ├─ screens        title → slots → story → stage intro → level → score tally → stage outro → map …
  │                 (one screen at a time in the screen container obf_5h; the API menu bar on top)
  └─ a level        obf_9k builds fresh managers, then every frame:
       GameControls.keyCheckPlayer   input → the player's actions
       CustomerChar.updateObject     the player: movement, skills, attacks, damage
       EnemyManager / ObjectManager / ItemManager / BulletManager / EffectManager
                                     each updates its list; every entity is a Character subclass
       EventManager                  broadcasts between objects and enemies (switches, doors, triggers)
       ChallengeManager              tallies for the level's challenges
       GameCam.adjustCamera          follows the player
       GameDisplay                   draws the tiles, the parallax and every entity
       GameHUD                       health, coins, weapon, challenge pop-ups
```

Every entity (player, enemy, object, item, bullet, effect) derives from `papaGame.models.Character`, which holds
the position, the tile collision and the animation cycles. The managers own the lists and talk to the level
through `obf_4R` and `GameDisplay`. Constants and per-class defaults come from the importer's tables
(`datamanager.json`, `enemies.json`, `objects.json`, `items.json`, `effects.json`, `customerchar.json`,
`challenges.json`), not from code.

## Systems

| system | classes | lines | done | what it does | needs |
|---|---|---|---|---|---|
| Game root | `§_-4R§` | 581 | 43 % | Owns the managers and the screens, starts and ends levels, picks the music. | everything |
| Data | `DataManager`, `LevelData`, `WorldData`, `ScreenData`, `CheckpointData`, `UserData`, `Challenge`, `CustomerData`, `CustomerDataFile` | 4 493 | 67 % | Tile and collision tables, worlds and rooms from `levels.xml`, checkpoints, save slots, challenge definitions, the customers' looks and skills. | importer tables |
| Customers | `CustomerAkari` … `CustomerZoe` (28) | 710 | 100 % | One small class per playable customer: outfits, weapon, skill. | `CustomerDataFile` |
| Display | `GameDisplay`, `GameCam` | 2 289 | 54 % | Builds and draws rooms (tiles, layers, parallax, moving backgrounds), the camera, room wipes. | data, `CustomerChar` |
| Input and events | `GameControls`, `EventManager` | 1 313 | 78 % | Keys to actions, pause and drop; the broadcast system that links switches, doors and enemies. | player |
| Managers | `BitmapManager`, `CharacterBlitter`, `EnemyManager`, `ObjectManager`, `ItemManager`, `BulletManager`, `EffectManager`, `MapManager`, `SoundManager`, `ChallengeManager` | 5 799 | 52 % | Lists of entities; sheets and puppet dressing; tile lookups; sound and music; challenge tallies and medals. | models |
| Base models | `Character`, `PlayerChar`, `Enemy`, `GameObject`, `GameItem`, `Bullet`, `Effect`, `GameHUD` | 5 093 | 69 % | Shared movement, collision, damage and animation; the HUD. | managers |
| Player | `CustomerChar` | 2 961 | 93 % | The playable customer: running, jumping, every skill, attacks with every weapon, grabbing, being hurt. | base models |
| Enemies | `Enemy1` … `Enemy39` | 13 106 | 10 % | One class per enemy type (see below). The biggest block left. | `Enemy`, bullets |
| Objects | `GameObject1` … `GameObject33` | 6 552 | 15 % | Platforms, blimps, berries, switches, doors, cages, crackers, cannons, checkpoints, boss triggers. | `GameObject`, events |
| Items | `GameItem_*` (10) | 642 | 84 % | Coins, hearts, balloon, fizzo can, gummy worm, gold helmet. | `GameItem` |
| Bullets | `Bullet`, 19 in `bullets/` | 7 887 | 0 % | Projectiles of the customers' weapons, the enemies and the cheese cannon. | `Character` |
| Effects | `Effect`, 49 in `effects/` | 5 026 | 37 % | Short animations: dust, bits, splats, sparkles, the cage, the boss escape. | `Effect` |
| Screens | 18 in `screens/` | 8 060 | 33 % | Every menu and transition screen (see below). | data, `UserData`, `CustomerData` |
| Flipline API | `§_-4B§.§_-5y§`, `§_-4B§.§_-r§`, `screens.§_-5d§`, `screens.§_-88§` | 639 | n/a | The menu bar, leaderboards and the preloader's play button. Not ported: `obf_4R::showApiScreen` and `engine::MenuBar` replace them. | — |

## Enemies

Class `EnemyN` is enemy type N in the levels' `enemyArray`; the names come from DataManager's `enemyKey`.
Subclasses only change a few defaults (`Enemy4` of `Enemy3`, `Enemy10`/`Enemy11` of `Enemy9`, `Enemy39` of
`Enemy38`).

| class | enemy | first world | done |
|---|---|---|---|
| Enemy1, 2 | Onion, Brown Onion | Leaf Landing | ✅ |
| Enemy6 | Red Tomato | Leaf Landing | ✅ |
| Enemy34 | Burgerzilla | Leaf Landing | ✅ |
| Enemy7, 12, 14 | Roma Tomato, Burger Slider, Lettuce Lark | Lettuce Lane | ✅ |
| Enemy3, 4, 8, 15, 16 | Army Onion (+4), Runt Tomato, Leafy Lark, Dill Wheel | Fort Onion | ✅ |
| Enemy5, 9, 10, 11, 17, 35 | Kaiser Onion, Party Sub (9–11), Dill Worm, Sarge (boss; fires `SargePepper`, `GiantPepper`) | Sarge's Lair |5, 9–11, 17 ✅ |
| Enemy13, 19, 25, 26, 32 | Brussel Lark, Blue Shroom, Swiss Zack, Cheddar Mack, Jellyback | Mount Monterey | |
| Enemy18, 21, 27, 31 | Dill Weed, Bow Shroom (fires `ShroomArrow`), Pepper Jack, Cheese Wheel | Cheddar Hills | |
| Enemy20, 22, 28, 29 | Thorn Shroom, Bacobar, Radish, Laddish | The Saucelands | |
| Enemy36, 37 | Radley's Hideout bosses: 36 (not in `enemyKey`; fires `Beaker`, boss music) and Radley Madish (rockets) | Radley's Hideout | |
| Enemy23, 30, 33 | Bacoburn, Pararadish, Awesome Saucer (fires `SauceBullet`) | BBQ Bog | |
| Enemy24, 38, 39 | Bacobites; 38/39 are not in `enemyKey` (38 fires the rockets) | no room places them: check how they spawn before translating | |

## Objects

| class | object | done |
|---|---|---|
| GameObject1 | Cage Pulley | ✅ |
| GameObject2 | Pushable Boulder | |
| GameObject3 | Ladder | ✅ |
| GameObject4–13 | Blimps: static, horizontal, vertical, circular, dropping (each with a small one) | 4–7 ✅ |
| GameObject14, 15, 16 | Drop Berry, On Berry, Off Berry | |
| GameObject17 | Switch | ✅ |
| GameObject18 | Door | ✅ |
| GameObject19, 20 | Cage, Cage Flipped | ✅ |
| GameObject21–25 | Cracker Blocks 1–5 | ✅ |
| GameObject26 | Cheese Cannon (fires `CheeseBullet`) | |
| GameObject27, 32 | BBQ Quicksauce, Rainbow Sauce | |
| GameObject28 | Checkpoint | ✅ |
| GameObject29, 30 | Start Fight, End Fight (boss arenas, boss music, the escape) | |
| GameObject31 | Spike Bed | |
| GameObject33 | Training Sensor | |

## Bullets

`BulletManager` keeps every type. Customer weapons: `PepperBomb` (Big Pauly), `Basketball` (Connor),
`BlueberryShot` (Penny), `DogToy` (Prudence), `Cookie` (Yippy), `GummyOnion` (Sarge Fan), `ChiliCan` (Rico),
`RomanCandle` (Boomer), `Banana` (Akari), `XandraStar` (Xandra). Enemies and objects: `ShroomArrow`, `SauceBullet`,
`CheeseBullet`, `Beaker` and `BeakerBlob`, `SargePepper`, `GiantPepper`, `RocketBBQ` and `RocketMustard`.

## Screens

| screen | when | done |
|---|---|---|
| SplashScreen | title | ✅ |
| SlotSelectScreen | choose or start a save slot | ✅ |
| GameIntroScreen | the story at the start of a new game | ✅ |
| StageIntroScreen | before each level | ✅ |
| MainMenuScreen | the map, the customer choice; also credits, baddies, badges, controls, help, pause and quit | map, customers, credits, pause (info, quit) |
| ScoreTallyScreen | level complete: score, coins, challenges, saving the result | ✅ |
| StageOutroScreen | before the tally: the customer and the rescued one fall through the warp | ✅ |
| NewCustomerScreen | a customer joins the team | ✅ |
| BossIntroScreen | before a boss fight | |
| TrainingPopup | the tutorial pop-ups | |
| ParadeScreen, GameOutroScreen | the ending | |
| LoadingLevelsScreen, FliplineLogoScreen, SponsorLogoScreen, LicenseLogoScreen | loader and logos | not needed |

## Order for what is left

Each step leaves the game playable further than before. The levels of a world need only the enemies and objects
of that world and the ones before it (tables above).

1. **Finish a level's loop:** done (tally, outro, new customer, pause, challenges, game over). Left: the drop-down
   that announces challenges and badges.
2. **Bullets:** `Bullet`, `BulletManager`, shooting and Connor's `Basketball` are done; next the other customers'
   weapons. Several enemies and objects need theirs.
3. **World by world**, in map order: Lettuce Lane → Fort Onion → Sarge's Lair (first boss: `BossIntroScreen`,
   `GameObject29/30`) → Mount Monterey → Cheddar Hills → The Saucelands → Radley's Hideout → BBQ Bog → X Zone.
   For each, its new enemies, objects and effects, then play every room with `--level`/`--room`.
4. **The rest of the menus:** baddies, badges, controls, help, the tutorial pop-ups, then the ending
   (`ParadeScreen`, `GameOutroScreen`).
5. **Training and challenges:** `GameObject33` and the remaining `Challenge` types.

PL3 shares 179 classes with PL2 (see AGENTS.md). Translating a PL2 class whole, with its QUIRKs, makes its PL3
counterpart mostly a diff.
