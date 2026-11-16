#pragma once
// AS: papaGame.screens.MainMenuScreen — the map of areas and, before a level, the customer and outfit choice,
// with the credits, badges, baddies, controls and help sections the bar's tabs open.
// The store, social and "first game" links went to web sites and are not shown.
// SKELETON: the info (pause), baddies, medals (badges), controls, help and confirm-quit sections, and the tutorial
// pop-ups (§_-6n§ / §_-c§).
#include <memory>
#include <string>
#include <vector>

#include "engine/movieclip.h"

namespace obf_9G { class obf_4R; }

namespace papaGame::screens {

// The API screen's parameters (`{section, useLevel, isCharSelect}`).
struct MainMenuParams {
    std::string section;
    double useLevel = -1;
    bool isCharSelect = false;
};

class MainMenuScreen {
public:
    MainMenuScreen(obf_9G::obf_4R *gameObj, engine::MovieClip *container, const MainMenuParams &params);

    void setupScreen();
    void setupCharacter(bool setup = true);
    void updateCharacter();
    void selectCharacter(double index, double style = 1);
    void updateStyleButtons();
    void clickCharacterStyle(double style);
    void clickCharacterThumb(double index);
    void setupMap(bool setup = true);
    void rolloverMapButton(int world, engine::MovieClip *worldClip);
    void rolloutMapButton();
    void clickPlayMap(int world);
    void clickStartLevel();
    void setupCredits();
    void destroy();
    void clickMap() { setSection("map"); }
    void clickCredits() { setSection("credits"); }
    void clickExit();
    void clickExitMapSelect();
    void clickBackToMap();
    void setSection(const std::string &section);
    void tweenSections();
    void startClosingScreen();
    void closeMainMenuScreen();
    std::string getSectionTitle() const;
    // Port: ENTER_FRAME of the screen (the section tween, the character run-in).
    void enterFrame();

    obf_9G::obf_4R *gameObj = nullptr;
    engine::MovieClip *container = nullptr;
    engine::MovieClip *clip = nullptr; // owned by the container
    MainMenuParams params;
    std::string currentSection, newSection;
    double tweenSpeed = 2;
    bool isTransitioning = false, isClosing = false, isStartingLevel = false, isOpeningMapDetail = false;
    double whichLevel = -1;
    bool isQuittingLevel = false, isContinuingToMap = false, isReturningToMap = false, isOnCharSelect = false;
    double numCharacters = 28;
    double selectedCharacterIndex = 0, selectedStyle = 1, lastCharacterIndex = 0;
    std::string lastCharacterName;
    engine::MovieClip *characterModel = nullptr; // owned by charholder
    std::vector<engine::MovieClip *> leavingModels;
    double characterStartX = 500, characterLeaveX = -100, characterTargetX = 210, characterTargetY = 80;
    double characterSpeed = 18;
    bool willRevealMap = false;
    double mapLastUnlocked = 0;

private:
    void buildModel(bool sameCharacter = false);
    engine::MovieClip *section(const std::string &name) { return clip->findChild(name); }
    bool tweening_ = false;     // port: the tweenSections ENTER_FRAME listener is on
    bool updatingChar_ = false; // port: the updateCharacter ENTER_FRAME listener is on
};

} // namespace papaGame::screens
