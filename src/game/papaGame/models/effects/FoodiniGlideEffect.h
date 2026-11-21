#pragma once
// AS: papaGame.models.effects.FoodiniGlideEffect — one of the six small balloons Foodini glides with: it eases to its
// spot above him and bobs; when the glide ends it floats away, or is reeled back in if balloons were let go a
// moment ago.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class FoodiniGlideEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void updateObject() override;
    bool checkOnScreen() const override;
    void floatTethered();
    const engine::Sheet *getSheet() const override;
    void redrawLine() const;
    void floatAway();

    AnimationCycle animCycleFrames = cycleType1("anim", 1, 0, 4, 4, 4); // one of the four colours, set at random
    double maxDist = 60;
    double playerAnchorOffset = -60;
    bool isVisible = true;
    double liftOffset = 0;
    double targetPadding = 5;
    double pathX = -1;
    double pathY = -1;
    double waveTimer = 0;
    double waveTimerMax = 80;
    double waveTimerOffset = 4;
    double stringCoordX = 12;
    double stringCoordY = 24;
    bool isAttached = true;
    bool isReelingIn = false;
    double tweenSpeed = 3;
    double riseDecay = 2;
};

} // namespace papaGame::models::effects
