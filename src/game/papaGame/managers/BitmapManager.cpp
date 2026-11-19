#include "game/papaGame/managers/BitmapManager.h"

#include <string>

namespace papaGame::managers {

namespace {
const char *nameAt(const std::array<const char *, 10> &names, double index)
{
    const auto i = static_cast<size_t>(index);
    return index >= 0 && i < names.size() ? names[i] : nullptr;
}
} // namespace

// AS: BitmapManager.BitmapManager
BitmapManager::BitmapManager(obf_9G::obf_4R *gameObj_, engine::SheetLibrary *sheets_)
    : gameObj(gameObj_), sheets(sheets_)
{
    // setupGameBitmaps(): the transition sheets come with the transitions.
}

// AS: BitmapManager.getTilesheetBitmap
const engine::Sheet *BitmapManager::getTilesheetBitmap(double which)
{
    if (whichTileset != which) {
        // createTilesheet(which): the five parts stacked into one 1984x1312 sheet.
        const char *name = nameAt(tilesetNames, which);
        tilesheet = name ? sheets->composeTilesheet(name) : nullptr;
        whichTileset = which;
    }
    return tilesheet;
}

// AS: BitmapManager.getMiddlegroundBitmap
const engine::Sheet *BitmapManager::getMiddlegroundBitmap(double which)
{
    if (whichTileset != which || mgsheet == nullptr) {
        const char *name = nameAt(mgNames, which);
        mgsheet = name ? sheets->get(std::string(name) + "_mg") : nullptr; // loadBitmapFromMovieClip
    }
    return mgsheet;
}

// AS: BitmapManager.getBackgroundBitmap
const engine::Sheet *BitmapManager::getBackgroundBitmap(double which)
{
    if (whichTileset != which || bgsheet == nullptr) {
        const char *name = nameAt(bgNames, which);
        bgsheet = name ? sheets->get(std::string(name) + "_bg") : nullptr; // loadBitmapFromMovieClip
    }
    return bgsheet;
}

} // namespace papaGame::managers
