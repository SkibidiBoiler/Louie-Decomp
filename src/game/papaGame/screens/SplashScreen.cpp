#include "game/papaGame/screens/SplashScreen.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::screens {

// AS: SplashScreen.SplashScreen
SplashScreen::SplashScreen(obf_9G::obf_4R *gameObj_, engine::MovieClip *container_)
    : gameObj(gameObj_), container(container_)
{
    setupScreen();
}

// AS: SplashScreen.setupScreen
void SplashScreen::setupScreen()
{
    clip = &container->addChild(engine::MovieClip::create(*gameObj->symbols, "splashScreenMC"), "");
    if (engine::MovieClip *iris = clip->findChild("iris")) {
        iris->gotoAndStop(1);
    }
    gameObj->addContainerListener("clickStart", [this] { clickStart(); });
    gameObj->addContainerListener("clickCredits", [this] { clickCredits(); });
    // "clickHighScores" opened the online leaderboard: not ported.
    if (engine::MovieClip *credits = clip->findChild("credits")) {
        credits->setY(416);
        credits->onClick = [this] { hideCredits(); };
    }
    if (gameObj->obf_7C->hasShownTitleScreen) {
        gameObj->obf_7C->playTrack("TitleTrack", 1, 0, "outin");
    } else {
        gameObj->obf_7C->playTrack("TitleTrack", 1, 0, "crossfade");
        gameObj->obf_7C->hasShownTitleScreen = true;
    }
    for (const char *promo : {"promo_holder", "promo2_holder", "promo3_holder"}) {
        if (engine::MovieClip *holder = clip->findChild(promo)) {
            holder->setVisible(false);
        }
    }
}

// AS: SplashScreen.animateScreen (ENTER_FRAME) — the credits panel slides up and down.
void SplashScreen::animateScreen()
{
    engine::MovieClip *credits = clip->findChild("credits");
    if (credits) {
        if (isShowingCredits && credits->y() > 0) {
            const double distance = credits->y();
            credits->setY(static_cast<float>(credits->y() - distance / creditsDivisor));
        } else if (!isShowingCredits && credits->y() < 416) {
            const double distance = 416 - credits->y();
            credits->setY(static_cast<float>(credits->y() + distance / creditsDivisor));
        }
    }
    engine::MovieClip *iris = clip->findChild("iris");
    if (iris && iris->currentFrame() + 1 == 21) {
        closeSplashScreen();
    }
}

// AS: SplashScreen.clickStart — the API calls (leaderboard, analytics, ads) are dropped.
void SplashScreen::clickStart()
{
    if (isShowingCredits) {
        hideCredits();
    }
    if (engine::MovieClip *iris = clip->findChild("iris")) {
        iris->gotoLabel("irisout", true);
    }
}

// AS: SplashScreen.clickCredits
void SplashScreen::clickCredits()
{
    isShowingCredits = !isShowingCredits;
}

// AS: SplashScreen.hideCredits
void SplashScreen::hideCredits()
{
    isShowingCredits = false;
}

// AS: SplashScreen.destroy
void SplashScreen::destroy()
{
    gameObj->removeContainerListener("clickStart");
    gameObj->removeContainerListener("clickCredits");
    container->removeChild(clip);
    clip = nullptr;
}

// AS: SplashScreen.closeSplashScreen — the API moves on to the slot select screen.
void SplashScreen::closeSplashScreen()
{
    gameObj->showApiScreen("SlotSelect");
}

} // namespace papaGame::screens
