#pragma once
// AS: papaGame.screens.GameIntroScreen — the story pages before a new game; Skip jumps to the first level.
#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class GameIntroScreen {
public:
    explicit GameIntroScreen(obf_9G::obf_4R *gameObj);

    void setupScreen();
    void updateScreen();
    void clickNext();
    void clickSkip();
    void finishGameIntro();
    void destroy();

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *clip = nullptr; // owned by the game's container
    double currentPage = 1;
    double totalPages = 8;
    bool didSkip = false;
};

} // namespace papaGame::screens
