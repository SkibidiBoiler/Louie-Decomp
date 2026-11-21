#pragma once
// AS: papaGame.models.effects.FoodiniBalloonEffect — Foodini's balloon: it floats on a string at most 60 px from his
// hand, bobbing, and leans with the string (one frame per 10 degrees); his attack hides it until the attack ends.
#include "game/papaGame/models/Effect.h"

namespace papaGame::display { class GameDisplay; }

namespace papaGame::models::effects {

class FoodiniBalloonEffect : public Effect {
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
    void updateAnimationFrame(double degrees);
    void resetBalloon();
    void floatAway();

    // The original's animCycleFrames (type 1); updateAnimationFrame sets its loop / first / last frame.
    AnimationCycle animCycleFrames = cycleType1("anim", 1, 0, 4, 4, 4);
    double maxDist = 60;
    double playerAnchorOffset = -30;
    bool isVisible = true;
    double liftOffset = 0;
    double targetPadding = 5;
    double pathX = -1;
    double pathY = -1;
    double waveTimer = 0;
    double waveTimerMax = 80;
    double waveTimerOffset = 4;
    std::vector<double> stringCoordsX = std::vector<double>(20, 27);
    std::vector<double> stringCoordsY = std::vector<double>(20, 25);
    bool isAttached = true;
};

// Port: the string from a balloon's knot to a point, as the original draws it on the screen bitmap: straight when it
// is longer than 0.7 * maxDist, else a quadratic curve (drawn here as short straight pieces).
void drawBalloonString(const display::GameDisplay &d, double fromX, double fromY, double toX, double toY,
                       double maxDist);

} // namespace papaGame::models::effects
