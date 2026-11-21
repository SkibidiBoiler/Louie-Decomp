#pragma once
// AS: papaGame.screens.BossIntroScreen — before Radley's Hideout is first beaten: Papa Louie, trapped, faces the
// radish boss over the hideout's backdrop while the mid-boss track plays; an iris closes it into the level.
#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class BossIntroScreen {
public:
    explicit BossIntroScreen(obf_9G::obf_4R *gameObj);

    void setupScreen();
    void updateScreen();
    void destroy();

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *clip = nullptr;   // owned by the game's container
    engine::MovieClip *papaMC = nullptr; // owned by clip.holder
    engine::MovieClip *mgMC = nullptr;   // owned by clip
    engine::MovieClip *bgMC = nullptr;   // owned by clip
    engine::MovieClip *bossMC = nullptr; // owned by clip.holder
    double animTimer = 0;
    double animTimerMax = 120;

private:
    std::unique_ptr<engine::MovieClip> buildModel(double whichCustomer, double whichStyle);
    void cleanupModel(engine::MovieClip *whichModel);
};

} // namespace papaGame::screens
