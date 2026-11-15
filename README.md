# Papa's Louie Recomp

Native port of a Papa Louie Flash game to **Windows** and **Android**, written in C++20 on top of SDL3.

This repository contains **no game code, assets or binaries**. You need your own copy of the original `.swf`;
the tools in `tools/` read it locally and the game loads the converted assets at runtime.

- Project rules, code profile and build commands: [AGENTS.md](AGENTS.md)
- Status: skeleton (engine loop and platform layers only, no game code yet).

## Getting the source

```bash
git clone --recursive https://github.com/SkibidiBoiler/Papas-Louie-Recomp.git
```

## License

MIT for the code in this repository (see [LICENSE](LICENSE)). The original game and its assets belong to their
owners and are not covered by this license.
