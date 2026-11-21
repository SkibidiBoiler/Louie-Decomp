#pragma once
// AS: papaGame.screens.FliplineLogoScreen — the Flipline Studios logo animation before the title screen, at 24 fps;
// at its "stopframe" label the game goes back to 30 fps and the title screen opens.
#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class FliplineLogoScreen {
public:
    explicit FliplineLogoScreen(obf_9G::obf_4R *gameObj);

    void setupScreen();
    void updateScreen();
    void destroy();

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *clip = nullptr; // owned by the game's container
};

} // namespace papaGame::screens
