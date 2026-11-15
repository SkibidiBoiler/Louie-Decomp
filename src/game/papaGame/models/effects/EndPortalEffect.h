#pragma once
// AS: papaGame.models.effects.EndPortalEffect — the portal the player drops out of at the start of a level (and enters at the end).
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class EndPortalEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void endAnimationCycle() override;
    void updateObject() override;
    bool checkOnScreen() const override;

    bool isClosed = false;
};

} // namespace papaGame::models::effects
