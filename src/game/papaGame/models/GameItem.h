#pragma once
// AS: papaGame.models.GameItem (extends Character) — coins, hearts and the special items to collect.
// SKELETON: dropped weapons (isWeapon), bouncing items.
#include <string>
#include <vector>

#include "game/as3.h"
#include "game/papaGame/models/Character.h"

namespace engine { struct JsonValue; }

namespace papaGame::models {

class GameItem : public Character {
public:
    // AS: GameItem(gameObj, xtile, ytile, id, whichScreen, extraParams, isDropped); the body runs in init().
    explicit GameItem(obf_9G::obf_4R *gameObj);
    virtual void init(double xtile, double ytile, double id, double whichScreen,
                      const std::vector<as3::Value> &extraParams, bool isDropped);
    // Port: the base class's cycle defaults from items.json.
    static void loadTables(const engine::JsonValue &base);

    std::vector<as3::Value> getItemData() const;
    bool canPickUp() const;
    virtual void setupParams(const std::vector<as3::Value> &params);
    virtual void collectItem();
    void placeObject(double tileX, double tileY, bool keepPosition = false);
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override {}
    bool checkOnScreen() const;
    void updateObject() override;

    // AS: `type`, a String in GameItem ("_Coin"); Character.type is a Number in the port.
    std::string itemType;
    double whichScreen = -1;
    bool background = false;
    bool foreground = false;
    double points = 100;
    double moneyValue = 1;
    double droppedTimer = 0;
    double timeBeforeFlicker = 150;
    double timeBeforeDisappear = 240;
    bool isFlickering = false;
    std::string weaponSKU;
    bool isWeapon = false;
    bool isEmpty = false;
    double remainingHealth = -1;
    double remainingAmmo = -1;
    AnimationCycle animCycleFrames, flickerCycleFrames;
    bool hasBounced = false;
    bool isDropped = false;
    bool doesMove = false;
    bool doesDisappear = false;
    bool shouldSaveOnRoomChange = false;

private:
    static CycleTable base_;
};

} // namespace papaGame::models
