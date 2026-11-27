#include "game/papaGame/models/GameObject.h"

#include <algorithm>
#include <cmath>

#include "engine/json.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::models {

CycleTable GameObject::base_;

void GameObject::loadTables(const engine::JsonValue &base)
{
    parseCycleTable(base, base_);
}

GameObject::GameObject(obf_9G::obf_4R *gameObj_) : Character(gameObj_)
{
    activeCycleFrames = base_["activeCycleFrames"];
    inactiveCycleFrames = base_["inactiveCycleFrames"];
    activatingCycleFrames = base_["activatingCycleFrames"];
    deactivatingCycleFrames = base_["deactivatingCycleFrames"];
}

// AS: GameObject constructor (Character's constructor calls defineVars first)
void GameObject::init(double xtile_, double ytile_, double id_, double whichScreen_, double screenObjID_,
                      const std::vector<as3::Value> &extraParams)
{
    defineVars();
    id = id_;
    whichScreen = whichScreen_;
    screenObjID = screenObjID_;
    setupParams(extraParams);
    placeObject(xtile_, ytile_);
    savePrevious(); // port: nothing to interpolate from yet
}

// AS: GameObject.placeObject — solid objects mark their tiles in the map.
void GameObject::placeObject(double tileX, double tileY)
{
    const display::GameDisplay &d = *gameObj->obf_71;
    xtile = tileX;
    ytile = tileY;
    x = xtile * d.tileWidth + d.tileWidth / 2;
    y = ytile * d.tileWidth + d.tileWidth / 2;
    initialY = y;
    if (!isActivated && isSolidInactive) {
        addCollision();
    } else if (isActivated && isSolidActive) {
        addCollision();
    }
    isPlaced = true;
}

// AS: GameObject.addCollision
void GameObject::addCollision()
{
    managers::MapManager &map = *gameObj->obf_4i;
    const double left = xtile + solidTilesRect.x, top = ytile + solidTilesRect.y;
    for (int row = 0; row < solidTilesRect.height; ++row) {
        for (int col = 0; col < solidTilesRect.width; ++col) {
            map.setProperty(left + col, top + row, solidType);
        }
    }
}

// AS: GameObject.removeCollision
void GameObject::removeCollision()
{
    managers::MapManager &map = *gameObj->obf_4i;
    const double left = xtile + solidTilesRect.x, top = ytile + solidTilesRect.y;
    for (int row = 0; row < solidTilesRect.height; ++row) {
        for (int col = 0; col < solidTilesRect.width; ++col) {
            map.unsetProperty(left + col, top + row, solidType);
        }
    }
}

// AS: GameObject.grabAnimationCycle
const AnimationCycle &GameObject::grabAnimationCycle()
{
    if (isActivating) {
        return activatingCycleFrames;
    }
    if (isDeactivating) {
        return deactivatingCycleFrames;
    }
    if (isActivated) {
        return activeCycleFrames;
    }
    return inactiveCycleFrames;
}

// AS: GameObject.endAnimationCycle
void GameObject::endAnimationCycle()
{
    if (cycleName.find("deactivating") != std::string::npos) {
        finishDeactivatingObject();
    } else if (cycleName.find("activating") != std::string::npos) {
        finishActivatingObject();
    }
}

// AS: GameObject.activateObject
void GameObject::activateObject()
{
    if (!isActivating) {
        isActivating = true;
        isDeactivating = false;
        isDeactivated = false;
    }
}

// AS: GameObject.deactivateObject
void GameObject::deactivateObject()
{
    if (!isDeactivating) {
        isDeactivating = true;
        isActivating = false;
        isActivated = false;
    }
}

// AS: GameObject.finishActivatingObject
void GameObject::finishActivatingObject()
{
    isActivating = false;
    isActivated = true;
    isDeactivated = false;
    isDeactivating = false;
    if (isSolidActive && !isSolidInactive) {
        addCollision();
    } else if (!isSolidActive && isSolidInactive) {
        removeCollision();
    }
    if (waitPanBack) {
        gameObj->obf_8o->setCameraPan(gameObj->obf_3Y->prepanx, gameObj->obf_3Y->prepany, "player", screenObjID);
        waitPanBack = false;
    }
    if (jigglecam) {
        gameObj->obf_3Y->setCameraJiggle(jiggleamount, jiggleduration);
    }
    if (broadcastAmount == 0) {
        broadcastAmount = broadcastAmountOn;
    }
    if (broadcastOn) {
        broadcastTriggered(broadcastAmount);
    } else {
        allowReactions();
    }
    broadcastAmount = 0;
    if (saveState) {
        saveObjectData();
    }
}

// AS: GameObject.finishDeactivatingObject
void GameObject::finishDeactivatingObject()
{
    isDeactivating = false;
    isDeactivated = true;
    isActivated = false;
    isActivating = false;
    if (!isSolidInactive && isSolidActive) {
        removeCollision();
    } else if (isSolidInactive && !isSolidActive) {
        addCollision();
    }
    if (waitPanBack) {
        gameObj->obf_8o->setCameraPan(gameObj->obf_3Y->prepanx, gameObj->obf_3Y->prepany, "player", screenObjID);
        waitPanBack = false;
    }
    if (jigglecam) {
        gameObj->obf_3Y->setCameraJiggle(jiggleamount, jiggleduration);
    }
    if (broadcastAmount == 0) {
        broadcastAmount = broadcastAmountOff;
    }
    if (broadcastOff) {
        broadcastTriggered(broadcastAmount);
    } else {
        allowReactions();
    }
    broadcastAmount = 0;
    if (saveState) {
        saveObjectData();
    }
}

// AS: GameObject.broadcastTriggered — tells the objects and enemies of its event.
void GameObject::broadcastTriggered(double amount, bool fromGlobal)
{
    if (tellTriggerArray.empty()) {
        allowReactions();
        return;
    }
    for (size_t i = 0; i < tellTriggerArray.size(); ++i) {
        const auto &t = tellTriggerArray[i];
        if (t.size() < 2) {
            continue;
        }
        const bool pan = tellTriggerPan != 0 && tellTriggerPanTarget == static_cast<double>(i) + 1;
        if (t[1] == 1) {
            gameObj->obf_7u->activateObject(whichScreen, t[0], pan, amount, fromGlobal);
        } else if (t[1] == 2) {
            gameObj->obf_E->activateObject(whichScreen, t[0], pan, amount, fromGlobal);
        }
    }
}

// AS: GameObject.sendTriggered — counts the triggers it receives and (de)activates on its receipt method.
void GameObject::sendTriggered(double amount, bool pan, bool fromGlobal)
{
    currentTriggers += amount;
    if (currentTriggers < 0) {
        currentTriggers = 0;
    }
    if (!canReact) {
        allowTriggerReactions();
        return;
    }
    auto activate = [&] {
        if (fromGlobal) {
            finishActivatingObject();
        } else if (pan) {
            setCameraPan("wait", "activate");
        } else {
            activateObject();
        }
    };
    auto deactivate = [&] {
        if (fromGlobal) {
            finishDeactivatingObject();
        } else if (pan) {
            setCameraPan("wait", "deactivate");
        } else {
            deactivateObject();
        }
    };
    const bool canActivate = !isActivating && !isActivated;
    const bool canDeactivate = !isDeactivating && !isDeactivated;
    if (currentTriggers == necessaryTriggers) {
        if (canActivate && (broadcastReceiptMethod == 1 || broadcastReceiptMethod == 3)) {
            activate();
        } else if (canDeactivate && (broadcastReceiptMethod == 2 || broadcastReceiptMethod == 3)) {
            deactivate();
        } else {
            allowTriggerReactions();
            // SKELETON: with a pan (not global), the camera pans back to the player here.
        }
    } else if (currentTriggers < necessaryTriggers) {
        if (canDeactivate && (broadcastReceiptMethod == 1 || broadcastReceiptMethod == 3)) {
            deactivate();
        } else if (canActivate && (broadcastReceiptMethod == 2 || broadcastReceiptMethod == 3)) {
            activate();
        } else {
            allowTriggerReactions();
        }
    } else {
        if (canDeactivate && broadcastReceiptMethod == 3) {
            deactivate();
        } else if (canActivate && broadcastReceiptMethod == 3) {
            activate();
        } else {
            allowTriggerReactions();
        }
    }
}

// AS: GameObject.setCameraPan — the camera pans here; the object reacts when it arrives (stopCameraPan).
void GameObject::setCameraPan(const std::string &wait, const std::string &reaction)
{
    if (!wait.empty()) {
        waitActivate = true;
        waitType = reaction;
    }
    gameObj->obf_8o->setCameraPan(x, y, "1", screenObjID);
}

// AS: GameObject.stopCameraPan — the camera got here: react, then pan back once the reaction is over.
void GameObject::stopCameraPan()
{
    if (waitActivate) {
        if (waitType == "activate") {
            activateObject();
        } else if (waitType == "deactivate") {
            deactivateObject();
        }
        waitPanBack = true;
        waitActivate = false;
    }
}

// AS: GameObject.setEventData
void GameObject::setEventData(const std::vector<std::vector<double>> &triggers, double pan, double panTarget)
{
    tellTriggerArray = triggers;
    tellTriggerPan = pan;
    tellTriggerPanTarget = panTarget;
}

// AS: GameObject.setEventTriggers
void GameObject::setEventTriggers(double necessary, const std::vector<std::vector<double>> &from)
{
    necessaryTriggers = std::trunc(necessary);
    fromTriggerArray = from;
}

// AS: GameObject.checkStartStepping — a character over the top of the object, falling (or walking on from another
// object), steps on it.
void GameObject::checkStartStepping(Character &c, bool alreadyOnObject)
{
    const double charRight = c.x + c.width;
    const double charLeft = c.x - c.width;
    const double right = x + collRect.x + collRect.width;
    const double left = x + collRect.x;
    if ((!isThrowing || hasLandedAfterThrown) && !isGrabbed) {
        if (charRight > left && charLeft < right) {
            const double above = y - c.y;
            const double reach = spriteTargetY + c.height - standOffset + 3;
            if (above <= reach && c.y < y - spriteTargetY + standOffset + 10) {
                if ((c.jump && c.jumpspeed >= 0) || (!c.jump && c.onMovingTile == false && alreadyOnObject)) {
                    stepOnObject(c);
                }
            }
        }
    }
}

// AS: GameObject.stepOnObject
void GameObject::stepOnObject(Character &c)
{
    if (!isDeactivated) {
        if (std::find(standingChars.begin(), standingChars.end(), &c) == standingChars.end()) {
            standingChars.push_back(&c);
            standingCharsLastX.push_back(c.x);
        }
        c.jump = false;
        c.onMovingTile = true;
        c.whichMovingTile = this;
    }
}

// AS: GameObject.stopStandingOnObject
void GameObject::stopStandingOnObject(Character &c)
{
    c.onMovingTile = false;
    c.whichMovingTile = nullptr;
    auto it = std::find(standingChars.begin(), standingChars.end(), &c);
    if (it != standingChars.end()) {
        standingCharsLastX.erase(standingCharsLastX.begin() + (it - standingChars.begin()));
        standingChars.erase(it);
    }
}

// AS: GameObject.checkStandingOnObject — off the sides, jumping up, grabbed or on a slope ends it.
bool GameObject::checkStandingOnObject(Character &c)
{
    bool standing = true;
    const double charRight = c.x + c.width;
    const double charLeft = c.x - c.width;
    const double right = x + collRect.x + collRect.width;
    const double left = x + collRect.x;
    if (charRight < left || charLeft > right) {
        standing = false;
    } else if (c.jump && c.jumpspeed < 0) {
        standing = false;
    } else if (isGrabbed) {
        standing = false;
    } else if (c.onSlope) {
        standing = false;
    }
    if (!standing) {
        stopStandingOnObject(c);
    }
    return standing;
}

// AS: GameObject.checkOnScreen
bool GameObject::checkOnScreen()
{
    return gameObj->obf_71->checkOnScreen(*this);
}

// AS: GameObject.updateObject
void GameObject::updateObject()
{
    if (gameObj->obf_71->checkOnScreen(*this)) {
        updateSprite();
    }
}

// AS: GameObject.acceptsDamage
bool GameObject::acceptsDamage(const std::string &kind, const std::string &weapon) const
{
    auto has = [&](const std::vector<std::string> &list) {
        return std::find(list.begin(), list.end(), "all") != list.end() ||
               std::find(list.begin(), list.end(), weapon) != list.end();
    };
    if (kind == "bullet" && canBeShot) {
        return has(shotByWeapons);
    }
    if (kind == "object" && canBeBrokenByObjects) {
        return has(brokenByObjects);
    }
    if (kind == "weapon" && canBeMeleed) {
        return has(meleedByWeapons);
    }
    return false;
}

} // namespace papaGame::models
