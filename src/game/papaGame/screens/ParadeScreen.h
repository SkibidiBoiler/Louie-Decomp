#pragma once
// AS: papaGame.screens.ParadeScreen — after the last boss: every unlocked customer marches across the screen in
// turn (running with their weapons, or walking and waving), then an iris closes into the game's ending.
#include <string>
#include <vector>

#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class ParadeScreen {
public:
    ParadeScreen(obf_9G::obf_4R *gameObj, bool showWeapons);

    void setupScreen();
    void updateScreen();
    void destroy();
    void releaseNextGroup();
    void setupCustomerGroups();
    std::string getCustomerClipName(double index) const;
    std::unique_ptr<engine::MovieClip> buildCustomer(double index);
    void clearCustomer(engine::MovieClip *customer);

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *clip = nullptr; // owned by the game's container
    bool didStart = false;
    double lastTime = 0;
    double customerSetsShown = 0;
    std::vector<double> allCustomerIDs;
    std::vector<std::string> allCustomerSkills;
    double whichOrder = -1;
    std::vector<std::string> paradeOrder{"all"};
    double walkSpeed = 2;
    double runSpeed = 4;
    std::vector<engine::MovieClip *> movingParts; // owned by clip.holder
    std::vector<std::string> movingPartsType;
    double startX = -20;
    double carStartY = 351;
    double customerStartY = 270;
    double customerScale = 0.21;
    bool showWeapons = false;

private:
    std::unique_ptr<engine::MovieClip> buildModel(double index);
    void cleanupModel(engine::MovieClip *model);
};

} // namespace papaGame::screens
