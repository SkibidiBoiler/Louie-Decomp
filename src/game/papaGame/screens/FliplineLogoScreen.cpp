#include "game/papaGame/screens/FliplineLogoScreen.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::screens {

// AS: FliplineLogoScreen.FliplineLogoScreen — the "FliplineLogoScreen" analytics event is dropped.
FliplineLogoScreen::FliplineLogoScreen(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupScreen();
}

// AS: FliplineLogoScreen.setupScreen — the logo plays at 24 fps. Its button opened flipline.com in the original
// (§_-36§); the port keeps no links to web sites, so the click does nothing.
void FliplineLogoScreen::setupScreen()
{
    clip = &gameObj->obf_5h->addChild(engine::MovieClip::create(*gameObj->symbols, "fliplineLogoMC"), "");
    gameObj->stageFrameRate = 24;
}

// AS: FliplineLogoScreen.updateScreen (ENTER_FRAME) — at "stopframe": back to 30 fps, the title screen, and this
// screen goes. The analytics events are dropped.
void FliplineLogoScreen::updateScreen()
{
    if (clip->currentFrameLabel() == "stopframe") {
        clip->stop();
        gameObj->stageFrameRate = 30;
        gameObj->showApiScreen("SplashScreen");
        gameObj->obf_4E(); // destroys this screen
    }
}

// AS: FliplineLogoScreen.destroy
void FliplineLogoScreen::destroy()
{
    gameObj->obf_5h->removeChild(clip);
    clip = nullptr;
}

} // namespace papaGame::screens
