#include "game/papaGame/models/objects/GameObject19.h"

#include "game/obf_9G/obf_4R.h"
#include "game/papaGame/models/effects/CageBackBarsEffect.h"
#include "game/papaGame/models/effects/CageFrontBarsEffect.h"
#include "game/papaGame/models/effects/CageSpinnerEffect.h"

namespace papaGame::models::objects {

// AS: GameObject19 constructor — the cage's bars, wheel and trapped customer are effects.
void GameObject19::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                        const std::vector<as3::Value> &extraParams)
{
    GameObject::init(xtile_, ytile_, id_, whichScreen_, screenObjID_, extraParams);
    managers::EffectManager &fx = *gameObj->obf_V;
    backBarsID = fx.addEffect(x + 4 + fx_offset, y + 32, "CageBackBarsEffect", fx_subtype);
    if (!gameObj->obf_51->hasCustomerUnlocked(customerActualIndex)) {
        // SKELETON: CageCustomerEffect (the trapped customer's puppet) is not ported; addEffect returns -1.
        customerFXID = fx.addEffect(x + 64, y + 128, "CageCustomerEffect", as3::String(cageCustomerIndex), false,
                                    facingDir);
    }
    frontBarsID = fx.addEffect(x + 4 + fx_offset, y, "CageFrontBarsEffect", fx_subtype);
    spinnerID = fx.addEffect(x + spinner_offset, y, "CageSpinnerEffect");
    playerEndX = x - spriteTargetX - 32;
}

// AS: GameObject19.defineVars
void GameObject19::defineVars()
{
    sheetname = "object_cage";
    sheetIsBitmap = false;
    objectname = "Cage";
    type = 19;
    spritewidth = 164;
    spriteheight = 192;
    spriteCenterX = 82;
    spriteCenterY = 96;
    spriteTargetX = 18;
    spriteTargetY = 16;
    sheetWidth = 1;
    sheetHeight = 1;
    collRect = {-16, -16, 16, 16};
    activeCycleFrames = cycleType1("active", 1, 0, 0, 0, 0);
    inactiveCycleFrames = cycleType1("inactive", 1, 0, 0, 0, 0);
    activatingCycleFrames = cycleType2("activating", 1, 0, -1, {0});
    deactivatingCycleFrames = cycleType2("deactivating", 1, 0, -1, {0});
    isSolidActive = true;
    isSolidInactive = false;
    solidTilesRect = {0, 0, 5, 6};
    solidType = "collision";
    isShootable = false;
    isActivated = true;
    isDeactivated = false;
    currentHealth = 3;
    maxHealth = 3;
    foreground = false;
    facingDir = -1;
    jigglecam = true;
    jiggleamount = 6;
    jiggleduration = 8;
}

// AS: GameObject19.setupParams — [which of the level's trapped customers]
void GameObject19::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        cageCustomerIndex = params[0].asNumber();
        customerActualIndex = gameObj->obf_3A->getTrappedCustomerIndex(gameObj->obf_8W, cageCustomerIndex);
    }
}

// AS: GameObject19.grabAnimationCycle
const AnimationCycle &GameObject19::grabAnimationCycle()
{
    return GameObject::grabAnimationCycle();
}

// AS: GameObject19.checkOnScreen
bool GameObject19::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 5 && xtile < d.currentXtileRight + 5;
}

// AS: GameObject19.updateObject
void GameObject19::updateObject()
{
    if (!isHit) {
        updateSprite();
    }
}

// AS: GameObject19.deactivateObject — "Deactivate!": the cage opens, the customer is free, the level ends.
void GameObject19::deactivateObject()
{
    managers::EffectManager &fx = *gameObj->obf_V;
    for (double effectID : {frontBarsID, backBarsID, spinnerID}) {
        if (effectID == -1) {
            continue;
        }
        Effect *e = fx.getEffect(effectID);
        if (!e) {
            continue;
        }
        if (e->effectType == "CageFrontBarsEffect") {
            static_cast<effects::CageFrontBarsEffect *>(e)->openCage();
        } else if (e->effectType == "CageBackBarsEffect") {
            static_cast<effects::CageBackBarsEffect *>(e)->openCage();
        } else if (e->effectType == "CageSpinnerEffect") {
            static_cast<effects::CageSpinnerEffect *>(e)->openCage();
        }
    }
    // SKELETON: the CageCustomerEffect's openCage (customerFXID).
    data::UserData &user = *gameObj->obf_51;
    user.savedCharacterIndex = customerActualIndex;
    user.savedCharacterWasUnlocked = user.hasCustomerUnlocked(customerActualIndex);
    user.unlockCustomer(customerActualIndex);
    // SKELETON: ChallengeManager.recordCustomerCage(cageCustomerIndex) and Xandra's tag in level 9.
    auto &p = *gameObj->playerObj;
    p.finishTargetX = playerEndX;
    p.finishCustomerFXIndex = customerFXID;
    p.finishLevel("cage");
    solidTilesRect = {0, 1, 5, 4};
    GameObject::deactivateObject();
}

// AS: GameObject19.destroy
void GameObject19::destroy()
{
    managers::EffectManager &fx = *gameObj->obf_V;
    for (double effectID : {frontBarsID, backBarsID, customerFXID, spinnerID, portalID}) {
        if (effectID != -1) {
            fx.removeEffect(effectID);
        }
    }
    GameObject::destroy();
}

// AS: GameObject19.addPortal
void GameObject19::addPortal()
{
    portalID = gameObj->obf_V->addEffect(x, y + spriteheight / 2, "EndPortalEffect");
}

} // namespace papaGame::models::objects
