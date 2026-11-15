#pragma once
// AS: papaGame.models.effects.CageSpinnerEffect — the wheel on top of a cage; openCage plays its opening.
#include "game/papaGame/models/Effect.h"

namespace papaGame::models::effects {

class CageSpinnerEffect : public Effect {
public:
    using Effect::Effect;

    void defineVars() override;
    const AnimationCycle &grabAnimationCycle() override;
    void updateObject() override;
    void openCage() { isOpen = true; } // AS: CageSpinnerEffect.openCage
    bool checkOnScreen() const override;

    bool isOpen = false;
};

} // namespace papaGame::models::effects
