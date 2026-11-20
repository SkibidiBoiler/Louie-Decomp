#pragma once
// AS: papaGame.models.GameObject (extends Character) — the base of GameObject1..GameObject33: ladders, blocks,
// checkpoints, cages, blimps, switches...
// SKELETON: grabbing / throwing / riding, pushing, stepping on objects, triggers between objects and camera pans.
#include <string>
#include <vector>

#include "game/as3.h"
#include "game/papaGame/models/Character.h"

namespace engine { struct JsonValue; }

namespace papaGame::models {

class GameObject : public Character {
public:
    // AS: GameObject(gameObj, xtile, ytile, id, whichScreen, screenObjID, extraParams). As with Enemy, the port
    // runs the constructor body in init() so that the subclass overrides are called.
    explicit GameObject(obf_9G::obf_4R *gameObj);
    virtual void init(double xtile, double ytile, double id, double whichScreen, double screenObjID,
                      const std::vector<as3::Value> &extraParams);
    // Port: the base class's cycle defaults from objects.json.
    static void loadTables(const engine::JsonValue &base);

    virtual void setupParams(const std::vector<as3::Value> & /*params*/) {}
    void placeObject(double tileX, double tileY);
    void addCollision();
    void removeCollision();
    virtual void saveObjectData() {}
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    virtual void activateObject();
    virtual void deactivateObject();
    virtual void finishActivatingObject();
    virtual void finishDeactivatingObject();
    void broadcastTriggered(double amount, bool fromGlobal = false);
    void sendTriggered(double amount, bool pan, bool fromGlobal = false);
    void setCameraPan(const std::string &wait, const std::string &reaction);
    void stopReactions() { canReact = false; }      // AS: GameObject.stopReactions
    void allowTriggerReactions() {}                  // AS: GameObject.allowTriggerReactions (empty)
    void setEventData(const std::vector<std::vector<double>> &triggers, double pan, double panTarget);
    void setEventTriggers(double necessary, const std::vector<std::vector<double>> &from = {});
    virtual void grabRideableObject(Character *c) { (void)c; isGrabRiding = true; } // AS: GameObject
    virtual void releaseRideableObject() { isGrabRiding = false; }                  // AS: GameObject
    virtual double getRideableDirection() const { return 1; }                         // AS: GameObject
    virtual void unregisterSpawn() {}
    virtual bool checkOnScreen();
    virtual void checkAdditionalBlitting(bool /*foreground*/, bool /*before*/ = false) {}
    virtual void updateTrainingSign() {} // AS: GameObject.updateTrainingSign (empty; no subclass overrides it)
    void allowReactions() { canReact = true; } // AS: GameObject.allowReactions
    void updateObject() override;
    virtual void getHit(double /*damage*/ = 1) {}
    bool acceptsDamage(const std::string &kind, const std::string &weapon) const;
    bool checkPushingObject() const { return isPushable && isBeingPushed; } // AS: GameObject.checkPushingObject
    void checkStartStepping(Character &c, bool alreadyOnObject = false);
    virtual void stepOnObject(Character &c);
    virtual void stopStandingOnObject(Character &c);
    virtual bool checkStandingOnObject(Character &c);

    std::string objectname;
    double whichScreen = -1;
    double screenObjID = -1;
    double originalScreenIndex = -1;
    double originalScreenObjectID = -1;
    bool isInteractive = true;
    bool isActivated = true;
    bool isDeactivated = false;
    bool isActivating = false;
    bool isDeactivating = false;
    bool canReactivate = true;
    bool isSolidActive = false;
    bool isSolidInactive = false;
    std::string solidType = "none";
    Rect solidTilesRect{0, 0, 2, 2};
    bool isGrabbable = false;
    bool isGrabbed = false;
    bool isPushable = false;
    bool isBeingPushed = false;
    bool saveDataAtCheckpoint = false; // read by ObjectManager.saveObjectsAtCheckpoint (not ported yet)
    AnimationCycle activeCycleFrames, inactiveCycleFrames, activatingCycleFrames, deactivatingCycleFrames;
    bool jigglecam = false;
    double jiggleamount = 10;
    double jiggleduration = 20;
    bool foreground = false;
    bool isPlaced = false;
    bool isShootable = false;
    bool blocksBullets = false;
    double throwSpeed = 4;
    double statAttack = 1;
    bool saveState = false;
    bool canReact = true;
    bool waitPanBack = false;
    bool waitActivate = false;
    std::string waitType;
    std::vector<std::vector<double>> tellTriggerArray;
    double tellTriggerPan = 0;
    double tellTriggerPanTarget = 0;
    std::vector<std::vector<double>> fromTriggerArray;
    bool isGrabRideable = false;
    bool isGrabRiding = false;
    bool delayRideableUntilPickup = false;
    double rideablePlayerOffsetX = 0;
    double rideablePlayerOffsetY = 0;
    bool canJumpFromRideable = true;
    double currentTriggers = 0;
    double necessaryTriggers = 1;
    double broadcastStyle = 1;
    bool broadcastOff = true;
    bool broadcastOn = false;
    double broadcastAmount = 0;
    double broadcastAmountOn = -1;
    double broadcastAmountOff = 1;
    double broadcastReceiptMethod = 2;
    bool canBeShot = false;
    bool canBeMeleed = false;
    bool canBeBrokenByObjects = false;
    std::vector<std::string> shotByWeapons{"all"};
    std::vector<std::string> meleedByWeapons{"all"};
    std::vector<std::string> brokenByObjects{"all"};
    bool canTravelBetweenRooms = true;
    bool isTilingObject = false;
    bool isLadder = false;
    double tilingHeight = 5;
    double topCapFrame = 0;
    double tilingFrame = 0;
    double bottomCapFrame = 0;
    double initialY = 0;
    bool isSteppable = false;
    bool isSteppableByObjects = false;
    std::vector<Character *> standingChars;
    std::vector<double> standingCharsLastX;
    double standOffset = 0;
    double standingWalkSpeed = 7;
    bool isThrowing = false;
    bool hasLandedAfterThrown = false;

private:
    static CycleTable base_;
};

} // namespace papaGame::models
