#include "game/papaGame/models/GameItem.h"

#include "engine/json.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models {

CycleTable GameItem::base_;

void GameItem::loadTables(const engine::JsonValue &base)
{
    parseCycleTable(base, base_);
}

GameItem::GameItem(obf_9G::obf_4R *gameObj_) : Character(gameObj_)
{
    animCycleFrames = base_["animCycleFrames"];
    flickerCycleFrames = base_["flickerCycleFrames"];
}

// AS: GameItem constructor (Character's constructor calls defineVars first)
void GameItem::init(double xtile_, double ytile_, double id_, double whichScreen_,
                    const std::vector<as3::Value> &extraParams, bool isDropped_)
{
    defineVars();
    id = id_;
    whichScreen = whichScreen_;
    x = -1;
    y = -1;
    isDropped = isDropped_;
    setupParams(extraParams);
    placeObject(xtile_, ytile_, x != -1 && y != -1);
    savePrevious(); // port: nothing to interpolate from yet
}

// AS: GameItem.getItemData — the row saved for the room: [name, x, y, (health), (ammo)].
std::vector<as3::Value> GameItem::getItemData() const
{
    std::vector<as3::Value> row{as3::Value::str(itemType.substr(1)), as3::Value::num(xtile), as3::Value::num(ytile)};
    if (remainingHealth > -1) {
        row.push_back(as3::Value::num(remainingHealth));
    }
    if (remainingAmmo > -1) {
        // QUIRK: with no remainingHealth, index 3 is left undefined and the ammo lands at index 4.
        if (row.size() < 4) {
            row.push_back(as3::Value::str("undefined"));
        }
        row.push_back(as3::Value::num(remainingAmmo));
    }
    return row;
}

// AS: GameItem.canPickUp
bool GameItem::canPickUp() const
{
    if (isWeapon) {
        return hasBounced && !isEmpty && gameObj->playerObj->duck;
    }
    return true;
}

// AS: GameItem.setupParams — [remaining health, remaining ammo] (dropped weapons)
void GameItem::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        remainingHealth = params[0].asNumber();
    }
    if (params.size() > 1) {
        remainingAmmo = params[1].asNumber();
    }
    if (remainingHealth == 0 || remainingAmmo == 0) {
        isEmpty = true;
    }
}

// AS: GameItem.collectItem (SKELETON: PlayerChar.addWeapon)
void GameItem::collectItem()
{
    gameObj->obf_3Z->removeItem(id);
}

// AS: GameItem.placeObject
void GameItem::placeObject(double tileX, double tileY, bool keepPosition)
{
    const display::GameDisplay &d = *gameObj->obf_71;
    if (!keepPosition) {
        xtile = tileX;
        ytile = tileY;
        x = xtile * d.tileWidth + d.tileWidth / 2;
        y = ytile * d.tileWidth + d.tileWidth / 2;
    }
    if (doesMove) {
        jump = true;
        jumpspeed = 0;
        fallChar();
    }
}

// AS: GameItem.grabAnimationCycle
const AnimationCycle &GameItem::grabAnimationCycle()
{
    return isFlickering ? flickerCycleFrames : animCycleFrames;
}

// AS: GameItem.checkOnScreen
bool GameItem::checkOnScreen() const
{
    return gameObj->obf_71->checkOnScreen(*this);
}

// AS: GameItem.updateObject
void GameItem::updateObject()
{
    const double lastJumpSpeed = jumpspeed;
    if (!isWeapon || isEmpty) {
        ++droppedTimer;
    }
    updateSprite();
    if (doesMove) {
        jumpChar();
        if (dirx != 0) {
            moveChar(dirx, 0, 0, true, true);
        }
        if (!hasBounced && (!jump || (lastJumpSpeed >= 0 && jumpspeed < 0))) {
            hasBounced = true;
            // AS: the dropped-weapon branch here is empty.
        }
    }
    if (doesDisappear) {
        if (droppedTimer >= timeBeforeDisappear) {
            gameObj->obf_3Z->removeItem(id);
        } else if (droppedTimer >= timeBeforeFlicker && !isFlickering) {
            isFlickering = true;
        }
    }
}

} // namespace papaGame::models
