#include "game/papaGame/models/objects/GameObject30.h"

#include <climits>

#include <SDL3/SDL.h>

#include "game/obf_9G/obf_4R.h"
#include "game/papaGame/models/effects/BossEscapeEffect.h"

namespace papaGame::models::objects {

// AS: GameObject30.defineVars
void GameObject30::defineVars()
{
    sheetname = "blanksprite_sheet";
    objectname = "End Fight";
    type = 30;
    spritewidth = 32;
    spriteheight = 128;
    spriteCenterX = 16;
    spriteCenterY = 64;
    spriteTargetX = 16;
    spriteTargetY = 16;
    sheetWidth = 1;
    sheetHeight = 1;
    collRect = {-16, -16, 32, 128};
    isActivated = true;
    foreground = true;
    broadcastStyle = 1;
    broadcastAmountOff = -1;
}

// AS: GameObject30.saveObjectData — QUIRK: isSpikes is not saved.
void GameObject30::saveObjectData()
{
    std::vector<as3::Value> row{as3::Value::num(type), as3::Value::num(xtile), as3::Value::num(ytile),
                                as3::Value::num(currentTriggers)};
    row.push_back(as3::Value::num(isActivated ? 1 : 0));
    row.push_back(as3::Value::num(shouldUnlockCamera ? 1 : 0));
    row.push_back(as3::Value::num(saveState ? 1 : 0));
    gameObj->obf_AQ->currentScreenData->saveObjectState(id - 1, row);
}

// AS: GameObject30.setupParams — [activated, unlock camera, save state, spikes]. QUIRK: an inactive one is not
// marked deactivated either.
void GameObject30::setupParams(const std::vector<as3::Value> &params)
{
    if (params.size() > 0) {
        isActivated = params[0].asNumber() == 1;
        isDeactivated = false;
    }
    if (params.size() > 1) {
        shouldUnlockCamera = params[1].asNumber() == 1;
    }
    if (params.size() > 2) {
        saveState = params[2].asNumber() == 1;
    }
    if (params.size() > 3) {
        isSpikes = params[3].asNumber() == 1;
    }
}

// AS: GameObject30.checkOnScreen — only the columns count (4 tiles of margin).
bool GameObject30::checkOnScreen()
{
    const display::GameDisplay &d = *gameObj->obf_71;
    return xtile > d.currentXtileLeft - 4 && xtile < d.currentXtileRight + 4;
}

// AS: GameObject30.updateObject
void GameObject30::updateObject()
{
    if (shouldChoreograph) {
        handleAnimation();
    }
    updateSprite();
}

// AS: GameObject30.finishDeactivatingObject — with spikes, the broadcast waits for the escape scene.
void GameObject30::finishDeactivatingObject()
{
    if (shouldUnlockCamera) {
        gameObj->obf_3Y->unlockCamera();
    }
    if (isSpikes) {
        shouldChoreograph = true;
    } else {
        GameObject::finishDeactivatingObject();
    }
}

// AS: GameObject30.reallyDeactivateObject
void GameObject30::reallyDeactivateObject()
{
    GameObject::finishDeactivatingObject();
}

// AS: GameObject30.handleAnimation — the escape scene: the player jumps to x 592 and waits, the floor shakes, the
// boss laughs, flies away by copter, and four blimps appear before the doors open.
void GameObject30::handleAnimation()
{
    characters::CustomerChar &p = *gameObj->playerObj;
    managers::EffectManager &fx = *gameObj->obf_V;
    if (animTimer == 0) {
        p.finishTargetX = 592;
        p.finishLevel("fake");
        waitingForPlayer = true;
        animTimer = 1;
    } else if (animTimer == 20) {
        gameObj->obf_3Y->setCameraJiggle(12, 12);
        p.jump = true;
        p.jumpspeed = -10;
        gameObj->obf_7C->playSound("papow.wav");
    } else if (animTimer == 40) {
        fx.addEffect(300, 3000, "LaughEffect", "B");
    } else if (animTimer == 42) {
        fx.addEffect(330, 2990, "LaughEffect", "W");
    } else if (animTimer == 44) {
        fx.addEffect(350, 2980, "LaughEffect", "A");
    } else if (animTimer == 50) {
        fx.addEffect(380, 2990, "LaughEffect", "H");
    } else if (animTimer == 52) {
        fx.addEffect(400, 2980, "LaughEffect", "A");
    } else if (animTimer == 57) {
        fx.addEffect(430, 2980, "LaughEffect", "H");
    } else if (animTimer == 59) {
        fx.addEffect(450, 2970, "LaughEffect", "A");
    } else if (animTimer == 64) {
        fx.addEffect(480, 2980, "LaughEffect", "H");
    } else if (animTimer == 66) {
        fx.addEffect(500, 2970, "LaughEffect", "A");
    } else if (animTimer == 68) {
        fx.addEffect(520, 2960, "LaughEffect", "Point");
    } else if (animTimer == 80) {
        escapeEffectID = fx.addEffect(400, 3000, "BossEscapeEffect");
        ++animTimer;
        gameObj->obf_7C->playTrack("MidBossTrack", INT_MAX);
    } else if (animTimer == 85) {
        gameObj->obf_3Y->setCameraJiggle(6, 6);
        gameObj->obf_7C->playSound("hit_switch.wav");
    } else if (animTimer == 120) {
        // [type, x tile, the blimp's second parameter]
        static const double blimps[4][3] = {{5, 18, 83}, {5, 15, 81}, {5, 12, 79}, {4, 8, 77}};
        managers::ObjectManager &objects = *gameObj->obf_7u;
        for (const auto &b : blimps) {
            objects.addObject(b[1], 70, b[0], whichScreen, static_cast<double>(objects.objects.size()) + 1,
                              {as3::Value::num(1), as3::Value::num(b[2])});
        }
    } else if (animTimer == 129) {
        reallyDeactivateObject();
    }
    if (!waitingForPlayer && escapeEffectID == -1) {
        ++animTimer;
    } else if (waitingForPlayer) {
        if (p.isFinishingLevel && p.finishJumps >= p.finishJumpsMax) {
            waitingForPlayer = false;
        }
    } else if (escapeEffectID != -1) {
        if (auto *escape = static_cast<effects::BossEscapeEffect *>(fx.getEffect(escapeEffectID))) {
            if (escape->isOut) {
                fx.removeEffect(escapeEffectID);
                escapeEffectID = -1;
            }
        }
    }
    if (animTimer == 130) {
        SDL_Log("FINISH ANIMATION, GET GOING!");
        p.isFinishingLevel = false;
        p.finishJumps = 0;
        gameObj->obf_8o->stopActionControls = false;
        isSpikes = false;
        gameObj->obf_7C->playTrack("BossTrack", INT_MAX);
    }
}

} // namespace papaGame::models::objects
