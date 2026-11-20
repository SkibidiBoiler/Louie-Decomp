#include "game/papaGame/managers/BitmapManager.h"

#include <string>

#include <SDL3/SDL.h>

#include "game/obf_9G/obf_4R.h"
#include "game/papaGame/managers/CharacterBlitter.h"

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

BitmapManager::~BitmapManager()
{
    destroy();
}

// AS: BitmapManager.destroy — frees the caged sheets.
void BitmapManager::destroy()
{
    for (engine::Sheet *s : {&caged1Sheet, &caged2Sheet, &caged3Sheet}) {
        if (s->texture) {
            SDL_DestroyTexture(s->texture);
        }
        *s = engine::Sheet{};
    }
}

// AS: BitmapManager.createCagedSprites — blits the trapped customer of cage 1, then 2 and 3 (worlds past 7 have
// one cage), and calls back. Port: the blitter finishes before returning, so the second half of finishedCageBlit
// (free the blitter, then the next cage or the callback) runs here, after each blit has returned.
void BitmapManager::createCagedSprites(const std::function<void()> &callback)
{
    const data::CustomerData &data = *gameObj->obf_3A;
    SDL_Log("Start Blitting Caged Sprites");
    cagedBlitCallback = callback;
    if (cagedBlitter) {
        cagedBlitter->destroy();
        cagedBlitter.reset();
    }
    cagedBlitter = std::make_unique<CharacterBlitter>(gameObj);
    blitWhichCage = 1;
    for (;;) {
        const std::string name =
            data.getCustomerClipName(data.getTrappedCustomerIndex(gameObj->obf_8W, blitWhichCage));
        cagedBlitter->createPlayerSprite([this] { finishedCageBlit(); }, true, name);
        cagedBlitter->destroy();
        cagedBlitter.reset();
        if (blitWhichCage == 3 || (blitWhichCage == 1 && gameObj->obf_8W > 7)) {
            if (cagedBlitCallback) {
                cagedBlitCallback();
            }
            return;
        }
        ++blitWhichCage;
        cagedBlitter = std::make_unique<CharacterBlitter>(gameObj);
    }
}

// AS: BitmapManager.finishedCageBlit (first half; see createCagedSprites). Port: torsoSheet.clone() takes the
// blitter's texture instead of copying it, since the blitter is destroyed straight after.
void BitmapManager::finishedCageBlit()
{
    engine::Sheet *target = blitWhichCage == 1 ? &caged1Sheet : blitWhichCage == 2 ? &caged2Sheet : &caged3Sheet;
    if (target->texture) {
        SDL_DestroyTexture(target->texture);
    }
    *target = cagedBlitter->torsoSheet;
    cagedBlitter->torsoSheet.texture = nullptr;
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
