#pragma once
// AS: papaGame.managers.BitmapManager — the runtime bitmaps. The port bakes them offline
// (tools/swf2assets.py --sheets) and loads them through engine::SheetLibrary.
// Ported so far: the tileset, backdrop (mg) and sky (bg) sheets.
#include <array>

#include "engine/sheets.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::managers {

class BitmapManager {
public:
    BitmapManager(obf_9G::obf_4R *gameObj, engine::SheetLibrary *sheets);

    const engine::Sheet *getTilesheetBitmap(double whichTileset);
    const engine::Sheet *getMiddlegroundBitmap(double whichTileset);
    const engine::Sheet *getBackgroundBitmap(double whichTileset);

    obf_9G::obf_4R *gameObj = nullptr;
    engine::SheetLibrary *sheets = nullptr; // port: owner of every baked sheet
    static constexpr std::array<const char *, 10> tilesetNames = {
        "tilesheet1", "tilesheet1", "tilesheet2", "tilesheet2", "tilesheet3",
        "tilesheet3", "tilesheet4", "tilesheet4", "tilesheet4", "tilesheet5"};
    static constexpr std::array<const char *, 10> mgNames = tilesetNames;
    static constexpr std::array<const char *, 10> bgNames = tilesetNames;
    const engine::Sheet *tilesheet = nullptr;
    const engine::Sheet *mgsheet = nullptr;
    const engine::Sheet *bgsheet = nullptr;
    double whichTileset = -1;
};

} // namespace papaGame::managers
