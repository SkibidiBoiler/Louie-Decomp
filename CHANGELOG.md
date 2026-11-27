# Changelog

## 0.2.0 — 2026-11-27

First release with downloads. **Only Papa Louie 2 is playable**; Papa Louie 3 and Papa Louie show as
"Coming soon" in the game picker. The downloads hold no game data: import your own `PapaLouie2.swf` in the app.

### Papa Louie 2

- All of the game code is translated (100 % of the functions `tools/progress.py` counts).
- Every world, from Leaf Landing to Radley's Hideout: Lettuce Lane, Fort Onion, Mount Monterey, Cheddar Hills, The
  Saucelands, X Zone and the rest, with their enemies, objects, moving platforms, switches, doors and blimps.
- Bosses: Sarge, the 9-B boss and Radley Madish with their intro screens.
- Every customer's weapon and bullet, rescues and the new-customer screen, checkpoints, the parade and the ending.
- Level-complete and score screens, game over, the pause menu with the BADDIES, BADGES, CONTROLS and HELP tabs.
- Challenges and badges: tallies, rewards, hidden badges and the drop-down announcements.
- The Flipline logo animation before the title screen.

### The port

- Game picker as the first screen, with a cover for each imported game.
- Options menu: frame-rate limit, smooth movement (interpolation), widescreen, language, touch-button size and
  transparency, FPS info.
- Widescreen: shows more of the level at the sides instead of black bars (optional).
- Menus in English and Spanish, with accented letters the original fonts lack.
- The importer draws morph shapes, aligns sprite sheets to whole pixels, repairs see-through tile edges and no
  longer leaves thin grey lines where white shapes meet (import the game again to get it).
- The map's help bubbles show below the top menu bar instead of hiding behind it.
- One `VERSION` file for Windows, Android and the picker; test runs save to their own slots.
- Project renamed to **Louie Attack Decomp**. Saves and settings from the earlier names move over by themselves.

### Downloads

- Windows: `LouieAttackDecomp-0.2.0-windows-x64.zip` (`louieattack.exe`, `SDL3.dll`, `pl_import.exe`). Windows 10
  or 11, 64-bit; no Visual C++ Redistributable needed.
- Android: `LouieAttackDecomp-0.2.0-android-arm64.apk`. Android 8.0 or newer on a 64-bit ARM phone. It is signed
  with a development key, so Android asks to allow installs from this source. It installs as a separate app from
  builds made before the rename (new app id `io.github.skibidiboiler.louieattack`).
