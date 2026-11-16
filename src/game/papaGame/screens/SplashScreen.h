#pragma once
// AS: papaGame.screens.SplashScreen — the title screen: the iris out when Play is pressed, the credits panel.
// The leaderboard, the promo links and the ads (§_-4Q§) belonged to the online API and are not ported.
#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class SplashScreen {
public:
    SplashScreen(obf_9G::obf_4R *gameObj, engine::MovieClip *container);

    void setupScreen();
    void animateScreen();
    void clickStart();
    void clickCredits();
    void hideCredits();
    void destroy();
    void closeSplashScreen();

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *container = nullptr;
    engine::MovieClip *clip = nullptr; // owned by the container
    double bgSpeed = 1;
    double mgSpeed = 2;
    bool isShowingCredits = false;
    double creditsDivisor = 4;
};

} // namespace papaGame::screens
