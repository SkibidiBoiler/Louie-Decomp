#pragma once
// AS: papaGame.screens.StageOutroScreen — after a level: the customer (and the one rescued from the cage, if new)
// falls through the warp with the coins and warp coins earned; then the level-complete tally (the API's EndOfDay).
#include <memory>

#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class StageOutroScreen {
public:
    explicit StageOutroScreen(obf_9G::obf_4R *gameObj);

    void setupScreen();
    void updateScreen();
    void destroy();

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *clip = nullptr;     // owned by the game's container
    engine::MovieClip *playerMC = nullptr; // owned by clip.inside
    engine::MovieClip *savedMC = nullptr;  // owned by clip.inside

private:
    std::unique_ptr<engine::MovieClip> buildModel(double whichCharacter, double style);
};

} // namespace papaGame::screens
