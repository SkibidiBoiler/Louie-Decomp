# Papas Louie Attack Decomp

Decompilation of every Papa's Louie Attack for Windows and Android: a native C++20 port (SDL3) of
*Papa Louie: When Pizzas Attack!*, *Papa Louie 2: When Burgers Attack!* and *Papa Louie 3: When Sundaes Attack!*.

This repository contains **no game code, assets or binaries**. You need your own copy of the original `.swf`
files; the tools in `tools/` read them locally and the game loads the converted assets at runtime.

- Project rules, code profile and build commands: [AGENTS.md](AGENTS.md)
- Asset format: [docs/asset-format.md](docs/asset-format.md) · Renderer notes: [docs/renderer.md](docs/renderer.md)
- Status: engine groundwork (asset extractor, timeline player, sheet blitting, interpolation/FPS options) running
  on Windows and Android with Papa Louie 2 data. No gameplay yet.

## Getting the source

```bash
git clone --recursive https://github.com/SkibidiBoiler/Papas-Louie-Recomp.git
```

## License

MIT for the code in this repository (see [LICENSE](LICENSE)). The original games and their assets belong to their
owners and are not covered by this license.
