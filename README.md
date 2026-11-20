# Papas Louie Attack Decomp

Decompilation of every Papa's Louie Attack for Windows and Android: a native C++20 port (SDL3) of
*Papa Louie: When Pizzas Attack!*, *Papa Louie 2: When Burgers Attack!* and *Papa Louie 3: When Sundaes Attack!*.

> **Unofficial fan project.** Not affiliated with, endorsed by or sponsored by Flipline Studios. *Papa Louie* and
> the game titles are trademarks of their owner and are used here only to say which games this port runs.
> Free and non-commercial: no donations, no paid builds. If you are the rights holder and want something changed
> or removed, please open an issue.

This repository contains **no game assets, original source files or binaries with game data**. You need your own
copy of the original `.swf` files. When you choose your copy, the app converts it locally, on your PC or phone:
graphics, sounds, levels and the game's data tables go to the app's data folder, and nothing is uploaded
anywhere. The game logic is not extracted from your copy either: it is rewritten in C++ and lives in
`src/game/`.

Please do not share converted data folders, `.swf` files or builds that bundle them.

- Project rules, code profile and build commands: [AGENTS.md](AGENTS.md)
- Asset format: [docs/asset-format.md](docs/asset-format.md) · Renderer notes: [docs/renderer.md](docs/renderer.md)
- Status: Papa Louie 2 is playable from the title screen: slot select and saving, the story, the map and the
  customer choice, and the levels with their enemies, items, objects, effects, bullets, HUD and sound. Still
  missing: the later worlds' enemies and objects, the boss, the level-complete score screen, the pause menu and
  some menu sections. `python -I tools/progress.py` shows how much of the original code is translated (about half
  of PL2). PL3 and PL1 come after PL2.
- Publishing checklist and legal notes: [docs/publishing.md](docs/publishing.md)

## Why a native port

The games were made for the Flash Player, which no longer exists. A native port runs them as ordinary programs,
with no Flash emulator and no browser.

✅ = working now · 🚧 = planned

**On both platforms**

- ✅ The game logic runs at its original speed (30 steps per second for PL2/PL3) whatever the frame rate.
- ✅ High frame rates: 60, 120 or the screen's maximum (120 by default), with optional interpolation so the
  motion is smooth instead of stepping at 30 FPS. "Original" mode shows the game exactly as it was.
- ✅ Sharp graphics at any resolution: the vector art is rasterised again at 2× (or more) instead of stretching
  small bitmaps.
- ✅ Works offline. No ads, no online services, no tracking.
- ✅ Imports your own copy inside the app: pick the game, choose its `.swf`, and the app converts it on the
  phone or the PC (graphics, sounds, levels and data tables; no Java, no extra tools, nothing sent online).
- ✅ Saved games on your device, three slots per game, as in the original.
- ✅ Keyboard and controller play through actions (left, jump, attack…) instead of fixed keys.
- ✅ Menus in English or Spanish (from the system language), including accented letters the original fonts lack.
- 🚧 Widescreen, more languages, and an options menu for all of these settings.
- 🚧 Remappable controls.

**On Windows**

- ✅ A small native `.exe` (plus `SDL3.dll`) that starts instantly; no Flash projector or old browser needed.
- ✅ Uses the full refresh rate of high-Hz monitors.
- 🚧 Resizable window and fullscreen at the monitor's own resolution.

**On Android**

- ✅ The games run as a real app: no Flash Player, no browser, full screen in landscape.
- ✅ Asks the phone for its highest refresh rate (tested at 120 Hz on a POCO X5 Pro).
- ✅ Hardware-accelerated rendering (OpenGL ES) for low battery use.
- ✅ Translucent on-screen touch buttons.
- 🚧 An editor for the touch controls: position, size, transparency and which buttons are shown, saved as
  profiles.
- 🚧 Physical controller support.

## Getting the source

```bash
git clone --recursive https://github.com/SkibidiBoiler/Papas-Louie-Decomp.git
```

If you cloned without `--recursive`, run `git submodule update --init --recursive` to fetch SDL3 and minimp3.

## Building

### Requirements

| for | you need |
|---|---|
| Windows | Visual Studio 2022+ or Build Tools (C++ workload, Windows SDK), CMake ≥ 3.24, Ninja |
| Android | Android SDK (platform 36), NDK, CMake 3.30 from the SDK, JDK 17+ |

### 1. Your copy of the game

Nothing to do before building: the first time the game starts it shows **Import a game**. Pick the game, choose
your `.swf` with the system file picker, and the app converts it into its data folder (about a minute on a PC, a
few minutes on a phone). The top band of the screen opens the importer again later.

The same converter runs from the command line on desktop, for checks or to prepare a data folder:

```bash
build/windows/pl_import.exe --swf "path/to/PapaLouie2.swf" --out "path/to/data"
```

`tools/swf2assets.py` (Python 3.10+, Java 17+ and [JPEXS FFDec](https://github.com/jindrapetrik/jpexs-decompiler)
26.x) is the older reference extractor and is no longer needed to play.

### 2. Windows

From a normal command prompt (the script loads the Visual Studio environment itself):

```bat
tools\build_windows.bat Debug
build\windows\papalouie.exe --data "path\to\data"
```

Use `Release` instead of `Debug` for an optimised build. Instead of `--data`, you can set the `PAPALOUIE_DATA`
environment variable or put a `data` folder next to the `.exe`.

Any other CMake setup also works:

```bash
cmake -S . -B build/windows -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build/windows
```

### 3. Android

Create `platform/android/local.properties` with `sdk.dir=C:/path/to/Android/Sdk` (forward slashes). Then:

```bash
cd platform/android
./gradlew assembleDebug
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

The APK holds no game data: copy your `.swf` to the phone (for example to *Download*) and import it from the
app. A data folder converted on a PC can also be copied to
`/sdcard/Android/data/io.github.skibidiboiler.papalouie/files/data/` with `adb push`.

## License

MIT for the code in this repository (see [LICENSE](LICENSE)). The original games and their assets belong to their
owners and are not covered by this license. Third-party code in `external/`: SDL3 (zlib), stb (MIT / public
domain), PlutoVG (MIT) and minimp3 (CC0).
