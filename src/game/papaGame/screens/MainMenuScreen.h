#pragma once
// AS: papaGame.screens.MainMenuScreen — the map of areas and, before a level, the customer and outfit choice,
// with the credits, badges, baddies, controls and help sections the bar's tabs open.
// The store, social and "first game" links went to web sites and are not shown.
// SKELETON: the help section, and the tutorial pop-ups (§_-6n§ / §_-c§).
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
    void setupMedals(bool setup = true);
    void setupBaddies(bool setup = true);
    void setupControls(bool setup = true);
    void clickControlsButton(const std::string &which);
    void cancelSettingControls();
    void controlsKeyListener(double keyCode);
    void clickControls() { setSection("controls"); } // AS: MainMenuScreen.clickControls
    std::string settingWhichKey = "none";
    void clickEnemyThumb(int index);
    void selectEnemy(double index);
    void clickBaddies() { setSection("baddies"); } // AS: MainMenuScreen.clickBaddies
    void populateMedals();
    void clickNextMedals();
    void clickPrevMedals();
    void animateMedalsTransition(); // the medals panel's ENTER_FRAME (port: from enterFrame)
    void clickMedals() { setSection("medals"); } // AS: MainMenuScreen.clickMedals
    void setupInfo(bool setup = true);
    void setupConfirmQuit(bool setup = true);
    void clickInfo() { setSection("info"); }
    void clickBackToGame();
    void clickQuit();
    void clickConfirmQuit();
    void clickCancelQuit();
    void destroy();
    void clickMap() { setSection("map"); }
    void clickCredits() { setSection("credits"); }
    void clickExit();
    void clickExitMapSelect();
    void clickBackToMap();
    void setSection(const std::string &section);
    void tweenSections();
    void showSectionTraining(); // port: part of tweenSections
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
    bool keysWereChanged = false;
    engine::MovieClip *infoModel = nullptr;        // owned by clip.info
    engine::MovieClip *confirmYesButton = nullptr; // owned by clip.confirmquit
    engine::MovieClip *confirmNoButton = nullptr;  // owned by clip.confirmquit
    double mapLastUnlocked = 0;

private:
    void buildModel(bool sameCharacter = false);
    std::unique_ptr<engine::MovieClip> buildInfoModel(double whichCharacter, double style);
    engine::MovieClip *section(const std::string &name) { return clip->findChild(name); }
    bool tweening_ = false;     // port: the tweenSections ENTER_FRAME listener is on
    bool updatingChar_ = false; // port: the updateCharacter ENTER_FRAME listener is on
    double medalsPage = 0;
    engine::MovieClip *enemyDetail = nullptr; // owned by the baddies section
    double medalsPerPage = 8;
    std::string medalsDirection = "next";
    bool medalsTransitioning = true;
    bool animatingMedals_ = false; // port: the animateMedalsTransition ENTER_FRAME listener is on
};

} // namespace papaGame::screens
