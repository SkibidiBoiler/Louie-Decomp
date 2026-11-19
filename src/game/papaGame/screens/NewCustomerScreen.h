#pragma once
// AS: papaGame.screens.NewCustomerScreen — after the tally, the customer rescued for the first time runs in with
// their name and skill; then the map opens.
#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class NewCustomerScreen {
public:
    NewCustomerScreen(obf_9G::obf_4R *gameObj, double whichCustomer);

    void setupScreen();
    void updateScreen();
    void destroy();

    obf_9G::obf_4R *gameObj = nullptr;
    double whichCustomer = 0;
    engine::MovieClip *clip = nullptr;  // owned by the game's container
    engine::MovieClip *model = nullptr; // owned by clip.charholder

private:
    void buildModel();
};

} // namespace papaGame::screens
