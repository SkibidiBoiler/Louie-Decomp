#include "game/papaGame/models/objects/GameObject21.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::models::objects {

// AS: GameObject21.defineVars
void GameObject21::defineVars()
{
    sheetname = "object_crackerblock1";
    sheetIsBitmap = false;
    objectname = "Cracker Block 1";
    type = 21;
    spritewidth = 32;
    spriteheight = 32;
    spriteCenterX = 16;
    spriteCenterY = 16;
    spriteTargetX = 16;
    spriteTargetY = 20;
    sheetWidth = 1;
    sheetHeight = 1;
    collRect = {-20, -20, 40, 40};
    activeCycleFrames = cycleType1("active", 1, 0, 0, 0, 0);
    inactiveCycleFrames = cycleType1("inactive", 1, 0, 4, 4, 4);
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {0});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {0});
    isSolidActive = true;
    isSolidInactive = false;
    solidTilesRect = {0, 0, 1, 1};
    solidType = "collision";
    isActivated = true;
    isShootable = false;
    isDeactivated = false;
    currentHealth = 1;
    maxHealth = 1;
    foreground = false;
    broadcastStyle = 2;
    broadcastReceiptMethod = 3;
    jigglecam = true;
    jiggleamount = 3;
    jiggleduration = 4;
    canBeShot = false;
    shotByWeapons = {"Bazooka", "CherryBomb", "Grenade", "TNT", "FuseBomb", "GrenadeLauncher", "HeatSeeker"};
    canBeMeleed = true;
    meleedByWeapons = {"Pound"};
    canBeBrokenByObjects = false;
    brokenByObjects = {"TNTBarrel", "RocketKeg", "SpikeBall", "TNTBundle"};
}

// AS: GameObject21.saveObjectData
void GameObject21::saveObjectData()
{
    std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                as3::Value::num(currentTriggers)};
    row.push_back(as3::Value::num(rememberBroken ? 1 : 0));
    row.push_back(as3::Value::num(currentHealth));
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row);
}

// AS: GameObject21.setupParams — [rememberBroken]
void GameObject21::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        rememberBroken = params[0].asNumber() == 1;
    }
}

// AS: GameObject21.grabAnimationCycle
const AnimationCycle &GameObject21::grabAnimationCycle()
{
    return activeCycleFrames;
}

// AS: GameObject21.getHit
void GameObject21::getHit(double damage)
{
    if (!isStunned) {
        currentHealth -= damage;
        if (currentHealth < 0) {
            currentHealth = 0;
        }
        // SKELETON: papow.wav when broken, impact_2.wav otherwise.
    }
    gameObj->obf_V->addEffect(x - 5, y - 5, "CrackerBreakEffect", "piece5");
    gameObj->obf_V->addEffect(x + 12, y + 2, "CrackerBreakEffect", "piece6");
    if (currentHealth <= 0) {
        canBeShot = false;
        canBeMeleed = false;
        canBeBrokenByObjects = false;
        blocksBullets = true;
        addBreakEffects();
        removeCollision();
        // SKELETON: ChallengeManager.recordTag("breakBlock").
        if (rememberBroken) {
            gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, {as3::Value::num(0)});
        }
        gameObj->obf_7u->removeObject(id);
    }
}

// AS: GameObject21.addBreakEffects
void GameObject21::addBreakEffects()
{
    managers::EffectManager &fx = *gameObj->obf_V;
    fx.addEffect(x - 8, y - 8, "CrackerBreakEffect", "piece1");
    fx.addEffect(x + 8, y - 8, "CrackerBreakEffect", "piece2");
    fx.addEffect(x - 8, y + 8, "CrackerBreakEffect", "piece3");
    fx.addEffect(x + 8, y + 8, "CrackerBreakEffect", "piece4");
    fx.addEffect(x - 7, y + 3, "CrackerBreakEffect", "piece5");
    fx.addEffect(x + 4, y - 2, "CrackerBreakEffect", "piece6");
}

// AS: GameObject21.checkOnScreen
bool GameObject21::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 3 && xtile < d.currentXtileRight + 3;
}

// AS: GameObject21.updateObject
void GameObject21::updateObject()
{
    if (!isHit) {
        updateSprite();
    }
}

} // namespace papaGame::models::objects
