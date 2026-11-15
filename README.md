# Papas Louie Attack Decomp

Decompilation of every Papa's Louie Attack for Windows and Android: a native C++20 port (SDL3) of
*Papa Louie: When Pizzas Attack!*, *Papa Louie 2: When Burgers Attack!* and *Papa Louie 3: When Sundaes Attack!*.

This repository contains **no game code, assets or binaries**. You need your own copy of the original `.swf`
files; the tools in `tools/` read them locally and the game loads the converted assets at runtime.

- Project rules, code profile and build commands: [AGENTS.md](AGENTS.md)
- Asset format: [docs/asset-format.md](docs/asset-format.md) · Renderer notes: [docs/renderer.md](docs/renderer.md)
- Status: engine groundwork (asset extractor, timeline player, sheet blitting, interpolation/FPS options) running
  on Windows and Android with Papa Louie 2 data. No gameplay yet.

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
- 🚧 Widescreen, translations into other languages, and an options menu for all of these settings.
- 🚧 Controller and keyboard support through remappable actions.

**On Windows**

- ✅ A small native `.exe` (plus `SDL3.dll`) that starts instantly; no Flash projector or old browser needed.
- ✅ Uses the full refresh rate of high-Hz monitors.
- 🚧 Resizable window and fullscreen at the monitor's own resolution.

**On Android**

- ✅ The games run as a real app: no Flash Player, no browser, full screen in landscape.
- ✅ Asks the phone for its highest refresh rate (tested at 120 Hz on a POCO X5 Pro).
- ✅ Hardware-accelerated rendering (OpenGL ES) for low battery use.
- 🚧 On-screen touch controls you can edit: position, size, transparency and which buttons are shown, saved as
  profiles.
- 🚧 Physical controller support.

## Getting the source

```bash
git clone --recursive https://github.com/SkibidiBoiler/Papas-Louie-Recomp.git
```

If you cloned without `--recursive`, run `git submodule update --init --recursive` to fetch SDL3.

## Building

### Requirements

| for | you need |
|---|---|
| asset extraction | Python 3.10+, Java 17+ (21 tested), [JPEXS FFDec](https://github.com/jindrapetrik/jpexs-decompiler) 26.x |
| Windows | Visual Studio 2022+ or Build Tools (C++ workload, Windows SDK), CMake ≥ 3.24, Ninja |
| Android | Android SDK (platform 36), NDK, CMake 3.30 from the SDK, JDK 17+ |

### 1. Convert your copy of the game

The game reads converted assets from a data folder. Keep that folder **outside** the repository.

```bash
python -I tools/swf2assets.py --swf "path/to/PapaLouie2.swf" --out "path/to/data/pl2" --scales 1,2 \
    --sheets tools/sheets/pl2.txt
```

Pass `--ffdec path/to/ffdec-cli.jar` and `--java path/to/java` if they are not in the default locations
(see `python tools/swf2assets.py --help`).
It writes `symbols.bin`, `manifest.json`, `levels.xml`, `shapes/`, `sheets/` and `sounds/`.

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

The APK holds no game data. Copy the converted assets to the app's storage (Git Bash users: run
`export MSYS_NO_PATHCONV=1` first):

```bash
R=/sdcard/Android/data/io.github.skibidiboiler.papalouie/files/data/pl2
adb shell mkdir -p $R/shapes $R/sheets
adb push path/to/data/pl2/symbols.bin path/to/data/pl2/manifest.json path/to/data/pl2/levels.xml $R/
adb push path/to/data/pl2/shapes/2x $R/shapes/
adb push path/to/data/pl2/sheets/2x path/to/data/pl2/sheets/index.txt $R/sheets/
```

## License

MIT for the code in this repository (see [LICENSE](LICENSE)). The original games and their assets belong to their
owners and are not covered by this license.
