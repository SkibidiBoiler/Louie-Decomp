# Publishing checklist and legal notes

Notes gathered on 2026-11-19 before making the repository public. Not legal advice.

## Where the risk is

- **Assets**: none in the repo or its history (audited on 2026-11-19: no `.swf`, images, sounds, level data or
  extracted scripts in any commit; `.gitignore` blocks them). Keep it that way.
- **Code**: `src/game/` is a C++ rewrite that follows the original ActionScript class by class. That makes it a
  derivative of the original code, the same situation as the decompilation projects of other games. Leaving the
  assets out lowers the risk but does not remove it.
- **Trademarks**: "Papa Louie" and the game titles belong to Flipline Studios. Use them only to say which games
  the port runs (README disclaimer), not as the project's brand.

### Precedents

| project | approach | outcome |
|---|---|---|
| Super Mario 64 / Ocarina of Time decompilations | matching decompilation, user supplies the ROM, no assets | still online; the rights holder acted against videos and builds that bundled assets, not against the code |
| re3 / reVC (GTA III, Vice City) | decompilation, user supplies the game, no assets | taken down by DMCA in 2021, then a lawsuit; some forks came back after counter-notices |
| OpenMW, OpenRA | clean-room engine that only reads the original data formats | not challenged |

What decides it is the rights holder's choice, not the method. GitHub removes a repository on a valid DMCA notice
without judging the claim; the owner can then file a counter-notice.

## Before going public

1. [ ] Ask Flipline Studios for permission (free, non-commercial, bring-your-own-copy, preservation after Flash).
   With a yes, publish; with a no, keep it private.
2. [ ] Pick a neutral repository and app name (the current repository, app id `io.github.skibidiboiler.papalouie`
   and `papalouie.exe` use the trademark). Renaming is the owner's decision.
3. [x] README: unofficial / not-affiliated disclaimer, no-donations note, contact for the rights holder.
4. [x] License for our own code: MIT ([LICENSE](../LICENSE)); third-party licenses listed in the README.
5. [x] History audit: no game files in any commit.
6. [ ] Never publish builds, release archives or data folders that contain converted game data, and never link to
   copies of the `.swf` files.
7. [ ] No donations or paid builds tied to the project.
8. [ ] Keep `.claude/skills/` out of git (third-party, no license; already ignored).

## Open-source tools worth knowing

| tool | license | use here |
|---|---|---|
| [Ruffle](https://ruffle.rs/compatibility) | MIT / Apache-2.0 | reference only: run the original `.swf` to compare behaviour with `--trace`. Never part of the port (rule 4). |
| [ax3](https://github.com/innogames/ax3) | MIT | AS3 → Haxe converter; could produce a first draft of a class to adapt to C++. Unmaintained, partial E4X. |
| [OpenFL](https://books.openfl.org/as3-conversion-guide/) + Haxe/hxcpp | MIT | an alternative whole route (AS3 → Haxe → native C++). Reimplements the Flash API, so it conflicts with rule 4; not used. |
| [Lightspark](https://en.wikipedia.org/wiki/Lightspark) | LGPL-3 | C++ Flash player with partial AS3. Reading only; copying code would bring LGPL terms. |
| [SWFRecomp](https://github.com/SWFRecomp/SWFRecomp) | MIT | static SWF → C recompiler. Only AS1/AS2 actions of SWF 4; its `DoABC` (AS3) case is commented out, so it cannot read PL2/PL3. |

No maintained AS3 → C++ translator was found, so the hand translation stays.
