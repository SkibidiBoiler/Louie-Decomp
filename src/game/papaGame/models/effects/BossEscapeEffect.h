#pragma once
// AS: papaGame.models.effects.BossEscapeEffect — the boss flying away by copter: it comes down to above the player,
// bobs there for a moment and flies off the top of the screen (then isOut is set).
#include <memory>

#include "game/papaGame/models/Effect.h"

namespace engine { class SoundChannel; }

namespace papaGame::models::effects {

class BossEscapeEffect : public Effect {
public:
    using Effect::Effect;

    void init(double x, double y, double id, const std::string &subtype, bool followPlayer, double playerXoffset,
              double playerYoffset) override;
    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    void destroy() override;

    double initialY = 0;
    double ydist = 330;
    double targetY = 0;
    double timeToRise = 35;
    double animTimer = 0;
    bool isTweeningToPlayer = true;
    double waitTimer = 0;
    double waitTimerMax = 30;
    bool isTweeningOut = false;
    double actualY = 0;
    bool isOut = false;
    double offScreenY = 0;
    double waveTimer = 0;
    double wavePeriod = 8;
    double waveAmplitude = 10;
    std::shared_ptr<engine::SoundChannel> copterSound;
};

} // namespace papaGame::models::effects
