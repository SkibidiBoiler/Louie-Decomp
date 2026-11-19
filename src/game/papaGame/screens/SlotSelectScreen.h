#pragma once
// AS: papaGame.screens.SlotSelectScreen — the three save slots, a new game's character (Marty or Rita) and name.
// The backup buttons and the "missing save" widget belonged to the online API and stay hidden; the name field
// (an fl.controls.TextInput) is drawn by the port and takes the keyboard's text.
#include <memory>
#include <string>

#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

class SlotSelectScreen {
public:
    SlotSelectScreen(obf_9G::obf_4R *gameObj, engine::MovieClip *container);

    void setupScreen();
    void keyListener(int key);
    void clickBack();
    void getSlotData();
    void clickNewSlot(double slot);
    void clickExistingSlot(double slot);
    void clickDeleteSlot(double slot);
    void clickDeleteYes(double slot);
    void clickDeleteNo();
    void clickEnterNameOK();
    void clickEnterNameCancel();
    void clickMarty();
    void clickRita();
    void showSlots();
    void showEnterName();
    void showCharacter();
    void animateScreen();
    void destroy();
    void startClosingScreen();
    void closeSlotSelectScreen();

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *container = nullptr;
    engine::MovieClip *clip = nullptr; // owned by the container
    double selectedSlot = 0;
    bool isClosing = false;
    bool isNewSlot = false;
    std::string currentMode = "slots";
    std::string selectedCharacter = "marty";
    std::string slotName;
    engine::MovieClip *model1 = nullptr, *model2 = nullptr, *model3 = nullptr; // owned by their holders
    engine::MovieClip *martyModel = nullptr, *ritaModel = nullptr;

private:
    std::unique_ptr<engine::MovieClip> buildModel(double whichCharacter, double style);
    engine::MovieClip *slotClip(int slot);
    void setInputText(const std::string &text);
    std::string inputText_; // port: the TextInput's text
    int caretTimer_ = 0;
};

} // namespace papaGame::screens
