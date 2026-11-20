#pragma once
// AS: papaGame.models.Effect (extends Character) — short animations: dust, stars, crumbs, the item pick-ups...
#include <string>

#include "game/papaGame/models/Character.h"

namespace engine { struct Sheet; }

namespace papaGame::models {

class Effect : public Character {
public:
    // AS: Effect(gameObj, x, y, id, subtype, followPlayer, playerXoffset, playerYoffset); the body runs in init().
    using Character::Character;
    virtual void init(double x, double y, double id, const std::string &subtype, bool followPlayer,
                      double playerXoffset, double playerYoffset);

    void placeEffect(double px, double py);
    void endAnimationCycle() override {}
    virtual bool checkOnScreen() const;
    void updateObject() override;
    virtual const engine::Sheet *getSheet() const;

    // AS: `type`, the class name as a String ("DustEffect"); Character.type is a Number in the port.
    std::string effectType;
    double whichScreen = -1;
    std::string subtype;
    bool followPlayer = false;
    double playerXoffset = 0;
    double playerYoffset = 0;
    bool foreground = false;
};

} // namespace papaGame::models
