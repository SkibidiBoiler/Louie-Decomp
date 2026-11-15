#pragma once
// AS: papaGame.models.Enemy (extends Character) — the base of Enemy1..Enemy39.
// SKELETON: multi-sprite enemies (updateMultiSprite), triggers to objects and camera pans, group messages.
#include <string>
#include <vector>

#include "game/as3.h"
#include "game/papaGame/models/Character.h"

namespace engine { struct JsonValue; }

namespace papaGame::models {

class Enemy : public Character {
public:
    // AS: Enemy(gameObj, xtile, ytile, id, whichScreen, whichTileset, screenObjID, extraParams). The port splits
    // the constructor: init() runs what the original's constructors run (defineVars, setupParams, placeEnemy),
    // since a C++ constructor cannot call the subclass overrides.
    explicit Enemy(obf_9G::obf_4R *gameObj);
    virtual void init(double xtile, double ytile, double id, double whichScreen, double whichTileset,
                      double screenObjID, const std::vector<as3::Value> &extraParams);
    // Port: the base class's constants (lookForPlayerRect, keepChasingPlayerRect) from enemies.json.
    static void loadTables(const engine::JsonValue &enemy);

    virtual void setupSprite() {}
    virtual void setupParams(const std::vector<as3::Value> & /*params*/) {}
    void placeEnemy(double tileX, double tileY);
    virtual void stunEnemy();
    virtual void reactToHurtingPlayer() {}
    virtual bool getHit(double damage = 1, double dir = 1, bool unused = false, const std::string &weapon = "none");
    void killMe(double dir = 1) override;
    const AnimationCycle &grabAnimationCycle() override;
    virtual void launchEnemy() {}
    virtual double getTorsoOffsetX() const { return 0; }
    virtual double getTorsoOffsetY() const { return 0; }
    void endAnimationCycle() override {}
    void broadcastTriggered(double state, bool fromGlobal = false);
    void sendTriggered(double state, bool pan, bool fromGlobal = false);
    virtual bool checkOnScreen();
    virtual void checkBeforeBlitting() {}
    void saveEnemyKill();
    virtual void removeEnemy();
    void jumpChar(bool collide = true) override;
    void updateObject() override;
    void destroy() override;
    virtual void startShoving() {}
    virtual void startQuickAttacking() {}
    virtual void startAttacking() {}
    void lookForPlayer();
    void setEventData(const std::vector<std::vector<double>> &triggers, double pan, double panTarget);
    void setEventTriggers(double necessary, const std::vector<std::vector<double>> &from = {});
    void updateSprite() override;
    double adjustSpeedForSlopes(double speed) const;
    virtual void receiveGroupMessage(const std::string & /*message*/, double /*value*/) {}

    double whichScreen = -1;
    double whichTileset = 0;
    double screenObjID = -1;
    bool foreground = false;
    bool canReact = true;
    bool canBeStunned = false;
    bool isUnstunning = false;
    bool isReactingToHurt = false;
    bool waitingToDie = false;
    bool resistBullets = false;
    bool waitActivate = false;
    bool waitPanBack = false;
    double currentTriggers = 0;
    double necessaryTriggers = 1;
    std::vector<std::vector<double>> tellTriggerArray;
    double tellTriggerPan = 0;
    double tellTriggerPanTarget = 0;
    std::vector<std::vector<double>> fromTriggerArray;
    double dieJumpSpeed = -24;
    double dieSpeed = 3;
    double hitsWhileDead = 0;
    double hitsToLaunch = 5;
    bool isLaunching = false;
    double launchJumpSpeed = -32;
    bool noticedPlayer = false;
    bool alwaysNoticePlayer = false;
    double statAttack = 1;
    bool hasDroppedWeapon = false;
    double dropWeaponChance = 1;
    std::string dropWeaponSKU;
    bool rememberKilled = true;
    double saveKillAsID = -1;
    bool onSingleTile = false;
    std::vector<std::string> usedWeapons;
    std::vector<std::string> usedWeaponsJuggling;
    std::string finalWeapon;
    bool isMultiSprite = false;
    std::string enemyGroupType;
    double strengthMultiplier = 1.5;
    double points = 100;
    Rect lookForPlayerRect;
    Rect keepChasingPlayerRect;
    bool canDealDamage = true;
    double stunBounceGravity = 2;
    double stunBounceJumpSpeed = -16;
    double shoveAmount = 0;
    bool bouncingDoesStun = true;
    bool canDodgeAttacks = false;
    double dodgeChance = 0.3;
    double attackChangeInRange = 0.03;
    bool isDodging = false;
    bool isAttacking = false;
    double spawnerObjectID = -1;
    double groupID = -1;
    std::string enemyCategory;
    bool didTagStun = false;

private:
    static Rect lookForPlayerRect_, keepChasingPlayerRect_;
};

} // namespace papaGame::models
