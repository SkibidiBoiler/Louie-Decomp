"""How much of a game's ActionScript has a C++ translation, by functions and by lines.

usage: python -I tools/progress.py [<deob scripts dir>] [--by-class] [--missing <Class>] [--excluded]

A function counts as translated when some file under src/game carries `// AS: <Class>.<function>` for it
(the tag every translated function has). Lines are the function's own lines in the decompiled code, without
blank or brace-only lines. Functions whose C++ is still marked SKELETON count as translated: the figure is an
upper bound. Functions in EXCLUDED are left out of the totals (the port drops or replaces them on purpose);
--excluded lists them. Reads the owner's local decompiled code (outside the repo); prints to the terminal only.
"""
import os
import re
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
DEFAULT_DEOB = "C:/Users/alfre/Proyectos/datos-papas/extracted/pl2-deob/scripts"
# The game's own code: papaGame.* and the obfuscated packages that belong to it. The rest of the SWF
# (Flipline API, ads, Mochi, Playtomic, timeline scripts) is not ported (AGENTS.md).
GAME_DIRS = ["papaGame", "§_-9G§", "§_-4B§"]

# What the port leaves out on purpose (AGENTS.md: online services, ads and sponsors are not ported; Flash's own
# loading and bitmap drawing are replaced by the importer and the port's renderer). Class -> "*" (the whole class)
# or a set of its functions, with the reason.
_ONLINE = "Flipline / sponsor / leaderboard / store / social links (online services, not ported)"
_PRELOAD = "Flash preloader, sponsor logos and loading screens (the port's own start-up replaces them)"
_BLIT = "Flash bitmap drawing (the port's renderer draws the level, sprites and effects)"
_LOAD = "Flash asset loading (the importer converts the assets ahead of time)"
_UNUSED = "never created by the game"
EXCLUDED = {
    "§_-5y§": ("*", _ONLINE),
    "§_-r§": ("*", _ONLINE),
    "§_-5d§": ("*", _PRELOAD),
    "§_-88§": ("*", _PRELOAD),
    "LoadingLevelsScreen": ("*", _PRELOAD),
    "FliplineLogoScreen": ({"§_-36§"}, _ONLINE),
    "LicenseLogoScreen": ("*", _PRELOAD),
    "SponsorLogoScreen": ("*", _PRELOAD),
    "RocketBBQ": ("*", _UNUSED),
    "RocketMustard": ("*", _UNUSED),
    "MainMenuScreen": ({"setupMapUpsell", "clickUpsellAppstore", "clickUpsellGooglePlay", "clickUpsellAmazon",
                        "clickUpsellKindle", "clickUpsellMoreInfo", "clickUpsellImage", "clickCreditsFlipline",
                        "clickCreditsPapaLouie", "clickFacebook", "clickTwitter", "clickBonusFacebook",
                        "clickBonusTwitter"}, _ONLINE),
    "GameHUD": ({"clickSponsorLogo", "clickLicenseLogo"}, _ONLINE),
    "SplashScreen": ({"clickHighScores", "closedHighScores", "errorHighScores", "clickPromo", "clickPromoTwo",
                      "clickPromoThree"}, _ONLINE),
    "ScoreTallyScreen": ({"clickSubmit", "leaderboardClosed"}, _ONLINE),
    "GameDisplay": ({"setupBitmapDraw", "switchRoomPreBlit", "reblitRectangle", "reblitAnimatedTile",
                     "blitLevelTiles", "blitTrimTiles", "blitOverlay", "blitEffectsOverlay", "reblitClouds",
                     "reblitLayer", "reblitCursor", "reblitSprite", "reblitMultispriteEnemy",
                     "blitCollisionArea"}, _BLIT),
    "BitmapManager": ({"loadBitmap", "loadMovieClip", "loadBitmapFromMovieClip"}, _LOAD),
    "DataManager": ({"getLeaderBoardID", "getMainLoaderInfo", "generateEnemyBitmap", "getEnemyBitmap"},
                    _ONLINE + "; " + _BLIT),
}

# Functions the game never calls: no call site (nor event listener) anywhere in the SWF's code. Found by searching
# every .as file for the name; a name shared with a called function elsewhere is not listed here.
_DEAD = "never called by the game (no call site in the SWF's code)"
DEAD = {
    "Challenge": {"meetsJuggleRequirements", "weaponIsFists", "meetsRescueRequirements"},
    "CustomerData": {"getCustomerFirstGame"},
    "DataManager": {"weaponNameToID", "levelDataError", "randomize", "getUpgradeCost", "getArsenalCost", "getNumberOfArsenal", "isArrow"},
    "ScreenData": {"createBackgroundBitmap"},
    "UserData": {"getLivesLostTally", "getTreasureBonus", "checkForCheats"},
    "GameCam": {"setCameraForceScrolling", "offsetCamera", "withinPanBounds"},
    "GameDisplay": {"getTileCoords"},
    "GameControls": {"playerHasDied", "setCameraAnchor"},
    "BitmapManager": {"createEnemySprite"},
    "EnemyManager": {"clearEnemySheet", "preblitEnemySheets", "getWeaponSheet", "clearWeaponSheets", "getCurrentEnemyTotal", "anyEnemiesAttacking", "checkForceNoDrop", "output"},
    "ItemManager": {"weaponNameToID", "dropEnemyWeapon"},
    "ObjectManager": {"getObjectByID"},
    "Character": {"checkForCloudToBottom"},
    "Enemy": {"setupSprite", "launchEnemy", "grabWeaponAnimationCycle", "startShoving", "startQuickAttacking"},
    "GameHUD": {"clickPause", "clickAdjustment", "animateTally", "tempRemoveInterface"},
    "GameObject": {"moveCarriedObject"},
    "PlayerChar": {"updateWeaponMastery", "checkIfCanGrab", "getPowerUp", "getInvincibility", "pauseAfterMiniboss", "beatMiniboss", "getWeaponSheetName", "getWeaponName"},
    "CustomerChar": {"checkForHittingBullets"},
    "§_-4R§": {"§_-m§", "§_-6P§", "§_-Z§"},
}

# Single functions left out for one of the reasons above, in classes that are otherwise ported.
_DEBUG = "the original's debug FPS / memory readout (the port has its own overlay)"
_INPUT = "Flash keyboard listeners and focus (the port's input layer sets the same key flags)"
EXCLUDED_FN = {
    ("LevelData", "prepareXML"): _LOAD, ("LevelData", "prepareXMLcycle"): _LOAD,
    ("LevelData", "getCharacter"): _LOAD, ("DataManager", "generateLevelData"): _LOAD,
    ("ScreenData", "createBitmap"): _BLIT, ("ScreenData", "clearBitmaps"): _BLIT,
    ("BitmapManager", "setupGameBitmaps"): _BLIT, ("BitmapManager", "setupTilesheets"): _BLIT,
    ("BitmapManager", "clearExtraSprites"): _BLIT, ("BitmapManager", "createTilesheet"): _BLIT,
    ("GameHUD", "clickFPS"): _DEBUG, ("GameHUD", "updateMessageFPS"): _DEBUG,
    ("GameControls", "adjustFramerate"): _DEBUG,
    ("GameControls", "setupControls"): _INPUT, ("GameControls", "resetKeyFocus"): _INPUT,
    ("GameControls", "keyDownListener"): _INPUT, ("GameControls", "keyUpListener"): _INPUT,
    ("SlotSelectScreen", "clickMissingWidget"): _ONLINE, ("UserData", "onFlushStatus"): _ONLINE,
    ("SoundManager", "apiMutedSound"): _ONLINE, ("SoundManager", "apiUnmutedSound"): _ONLINE,
}


def is_excluded(cls, name):
    e = EXCLUDED.get(cls)
    return (e is not None and (e[0] == "*" or name in e[0])) or name in DEAD.get(cls, ()) or (cls, name) in EXCLUDED_FN


def exclusion_reason(cls, name):
    e = EXCLUDED.get(cls)
    if e is not None and (e[0] == "*" or name in e[0]):
        return e[1]
    return EXCLUDED_FN.get((cls, name), _DEAD)

HEAD = re.compile(r"\s*(override\s+)?(public|private|protected|internal)?\s*(static\s+)?"
                  r"function\s+(?:(get|set)\s+)?([^\s(]+)\s*\(")
TAG = re.compile(r"//\s*AS:\s*([^\s—(,;]+)")


def as_functions(path):
    """Yield (name, line count) for each function of one .as file."""
    lines = open(path, encoding="utf-8").read().split("\n")
    starts = [(i, m.group(5)) for i, l in enumerate(lines) if (m := HEAD.match(l))]
    for k, (i, name) in enumerate(starts):
        end = starts[k + 1][0] if k + 1 < len(starts) else len(lines)
        body = [l for l in lines[i:end] if l.strip() not in ("", "{", "}", "};")]
        yield name, len(body)


def cpp_tags():
    tags = set()
    for d, _, files in os.walk(os.path.join(ROOT, "src", "game")):
        for f in files:
            if f.endswith((".cpp", ".h")):
                for m in TAG.finditer(open(os.path.join(d, f), encoding="utf-8").read()):
                    parts = m.group(1).split(".")
                    if len(parts) >= 2:
                        tags.add((parts[-2], parts[-1]))
                    # `// AS: pkg.Class` heads a class file; it covers the constructor.
                    tags.add((parts[-1], parts[-1]))
    return tags


def main():
    sys.stdout.reconfigure(encoding="utf-8")
    args = sys.argv[1:]
    deob = args.pop(0) if args and not args[0].startswith("--") else DEFAULT_DEOB
    by_class = "--by-class" in args
    missing = args[args.index("--missing") + 1] if "--missing" in args else None
    list_excluded = "--excluded" in args
    tags = cpp_tags()
    ex_f = ex_l = 0

    rows = []  # (class, done functions, functions, done lines, lines)
    for g in GAME_DIRS:
        for d, _, files in os.walk(os.path.join(deob, g)):
            for f in sorted(files):
                if not f.endswith(".as"):
                    continue
                cls = f[:-3]
                allfn = list(as_functions(os.path.join(d, f)))
                fn = [(name, n) for name, n in allfn if not is_excluded(cls, name)]
                for name, n in allfn:
                    if is_excluded(cls, name):
                        ex_f += 1
                        ex_l += n
                        if list_excluded:
                            print(f"  {cls}.{name}  ({n} lines): {exclusion_reason(cls, name)}")
                if missing == cls:
                    for name, n in fn:
                        if (cls, name) not in tags:
                            print(f"  {name}  ({n} lines)")
                    return
                if not fn:
                    continue
                done = [(name, n) for name, n in fn if (cls, name) in tags]
                rows.append((cls, len(done), len(fn), sum(n for _, n in done), sum(n for _, n in fn)))

    if by_class:
        for cls, df, f, dl, l in sorted(rows, key=lambda r: r[3] / r[4] if r[4] else 1):
            print(f"{100 * dl / l if l else 100:5.1f} %  {df:3}/{f:<3} functions  {cls}")
    df = sum(r[1] for r in rows)
    f = sum(r[2] for r in rows)
    dl = sum(r[3] for r in rows)
    l = sum(r[4] for r in rows)
    started = sum(1 for r in rows if r[1])
    print(f"classes:   {started} of {len(rows)} started")
    print(f"functions: {df} of {f} translated ({100 * df / f:.1f} %)")
    print(f"lines:     {dl} of {l} ({100 * dl / l:.1f} %)")
    print(f"excluded:  {ex_f} functions, {ex_l} lines (dropped or replaced on purpose; --excluded lists them)")


if __name__ == "__main__":
    main()
