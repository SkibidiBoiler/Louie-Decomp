#include "game/papaGame/screens/GameIntroScreen.h"

#include <climits>
#include <string>

#include "game/obf_9G/obf_4R.h"

namespace papaGame::screens {

// AS: GameIntroScreen.GameIntroScreen
GameIntroScreen::GameIntroScreen(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupScreen();
}

// AS: GameIntroScreen.setupScreen
void GameIntroScreen::setupScreen()
{
    clip = &gameObj->obf_5h->addChild(engine::MovieClip::create(*gameObj->symbols, "gameIntroMC"), "");
    gameObj->obf_7C->endMusic();
    gameObj->obf_7C->playTrack("TitleTrack");
    if (engine::MovieClip *skip = clip->findChild("skip_btn")) {
        skip->onClick = [this] { clickSkip(); };
    }
    for (const char *panel : {"panel1", "panel2", "panel3"}) {
        if (engine::MovieClip *p = clip->findChild(panel)) {
            p->gotoAndStop(1);
        }
    }
}

// AS: GameIntroScreen.updateScreen (ENTER_FRAME) — at the end of the clip, the next page or the end.
void GameIntroScreen::updateScreen()
{
    if (clip->currentFrame() + 1 == clip->frameCount()) {
        if (currentPage < totalPages) {
            ++currentPage;
            for (const char *panel : {"panel1", "panel2", "panel3"}) {
                if (engine::MovieClip *p = clip->findChild(panel)) {
                    p->gotoAndStop(static_cast<int>(currentPage));
                }
            }
            clip->gotoAndPlay(1);
            if (currentPage == 6) {
                gameObj->obf_7C->playTrack("BossTrack", INT_MAX, 0, "crossfade");
            }
        } else {
            finishGameIntro();
        }
    }
}

// AS: GameIntroScreen.clickNext (no listener calls it in this version)
void GameIntroScreen::clickNext()
{
    if (engine::MovieClip *next = clip->findChild("next_btn")) {
        next->setVisible(false);
    }
    if (engine::MovieClip *page = clip->findChild("page" + std::to_string(static_cast<int>(currentPage)))) {
        page->gotoLabel("ender", true);
    }
}

// AS: GameIntroScreen.clickSkip
void GameIntroScreen::clickSkip()
{
    didSkip = true;
    clip->stop();
    if (engine::MovieClip *skip = clip->findChild("skip_btn")) {
        skip->setVisible(false);
    }
    finishGameIntro();
}

// AS: GameIntroScreen.finishGameIntro — level 1 (world 0): fresh managers, which start its stage intro.
void GameIntroScreen::finishGameIntro()
{
    gameObj->obf_8W = 0;
    gameObj->obf_9k();
    gameObj->obf_5a();
}

// AS: GameIntroScreen.destroy
void GameIntroScreen::destroy()
{
    gameObj->obf_5h->removeChild(clip);
    clip = nullptr;
}

} // namespace papaGame::screens
