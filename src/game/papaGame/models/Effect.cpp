#include "game/papaGame/models/Effect.h"

#include <cmath>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models {

// AS: Effect constructor (Character's constructor calls defineVars first)
void Effect::init(double x_, double y_, double id_, const std::string &subtype_, bool followPlayer_,
                  double playerXoffset_, double playerYoffset_)
{
    defineVars();
    id = id_;
    subtype = subtype_;
    followPlayer = followPlayer_;
    playerXoffset = playerXoffset_;
    playerYoffset = playerYoffset_;
    placeEffect(x_, y_);
    savePrevious(); // port: nothing to interpolate from yet
    if (followPlayer && gameObj->playerObj) {
        // Port: the item pick-ups are added at (0, 0) and move onto the player in their first updateObject; the
        // interpolated render must not start from (0, 0), or they flash in the screen's top-left corner.
        prevX = gameObj->playerObj->x + playerXoffset;
        prevY = gameObj->playerObj->y + playerYoffset;
    }
}

// AS: Effect.placeEffect
void Effect::placeEffect(double px, double py)
{
    const double tw = gameObj->obf_71->tileWidth;
    x = px;
    y = py;
    xtile = std::floor(x / tw);
    ytile = std::floor(y / tw);
}

// AS: Effect.checkOnScreen
bool Effect::checkOnScreen() const
{
    return gameObj->obf_71->checkOnScreen(*this);
}

// AS: Effect.updateObject
void Effect::updateObject()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (d.checkOnScreen(*this)) {
        if (followPlayer) {
            x = gameObj->playerObj->x + playerXoffset;
            y = gameObj->playerObj->y + playerYoffset;
            xtile = std::floor(x / d.tileWidth);
            ytile = std::floor(y / d.tileWidth);
        }
        updateSprite();
    } else if (xtile < d.currentXtileLeft - 4) {
        gameObj->obf_V->removeEffect(id);
    } else if (y > d.screenPxHeight * 2) {
        gameObj->obf_V->removeEffect(id);
    }
}

// AS: Effect.getSheet
const engine::Sheet *Effect::getSheet() const
{
    const auto &sheets = gameObj->obf_V->objectSheets;
    const auto i = static_cast<size_t>(sheetIndex);
    return sheetIndex >= 0 && i < sheets.size() ? sheets[i] : nullptr;
}

} // namespace papaGame::models
