#include "game/papaGame/screens/StageIntroScreen.h"

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::screens {

// AS: StageIntroScreen.StageIntroScreen
StageIntroScreen::StageIntroScreen(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupScreen();
}

// AS: StageIntroScreen.setupScreen
void StageIntroScreen::setupScreen()
{
    clip = &gameObj->obf_5h->addChild(engine::MovieClip::create(*gameObj->symbols, "stageIntroMC"), "");
    gameObj->obf_7C->endMusic();
    gameObj->obf_7C->playSound("startofday.wav");
    clip->setPosition(350, 208);
    std::unique_ptr<engine::MovieClip> model =
        buildModel(gameObj->obf_51->selectedCharacter, gameObj->obf_51->selectedStyle);
    if (model) {
        model->gotoAndStop(1);
        model->gotoLabel("fall", false);
        if (engine::MovieClip *inside = clip->findChild("inside")) {
            playerMC = &inside->addChild(std::move(model), "");
        }
    }
}

// AS: StageIntroScreen.updateScreen (ENTER_FRAME)
void StageIntroScreen::updateScreen()
{
    ++delayTimer;
    const int frame = clip->currentFrame() + 1, total = clip->frameCount();
    if (frame == total && playerReady && cagesReady) {
        gameObj->obf_6V(gameObj->obf_8W);
        gameObj->obf_q(); // destroys this screen
    } else if (frame == total - 1 && !playerReady) {
        // Port: createPlayerSprite finishes at once and calls back.
        if (!gameObj->playerObj->blitter) {
            gameObj->playerObj->blitter = std::make_unique<managers::CharacterBlitter>(gameObj);
        }
        gameObj->playerObj->blitter->createPlayerSprite([this] { spriteReady(); });
    }
}

// AS: StageIntroScreen.spriteReady — SKELETON: BitmapManager.createCagedSprites (the trapped customers' sheets).
void StageIntroScreen::spriteReady()
{
    SDL_Log("SPRITE READY, load caged sprites:");
    playerReady = true;
    cagedSpritesReady();
}

// AS: StageIntroScreen.cagedSpritesReady
void StageIntroScreen::cagedSpritesReady()
{
    SDL_Log("CAGED SPRITES READY");
    cagesReady = true;
}

// AS: StageIntroScreen.destroy
void StageIntroScreen::destroy()
{
    if (playerMC) {
        if (engine::MovieClip *inside = clip->findChild("inside")) {
            inside->removeChild(playerMC);
        }
        playerMC = nullptr;
    }
    gameObj->obf_5h->removeChild(clip);
    clip = nullptr;
}

// AS: StageIntroScreen.buildModel — the customer puppet (customerTrappedMC) dressed with the customer's parts.
// SKELETON: the black GlowFilter outline (filters are not drawn yet).
std::unique_ptr<engine::MovieClip> StageIntroScreen::buildModel(double whichCharacter, double style)
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    std::unique_ptr<engine::MovieClip> model = engine::MovieClip::create(symbols, "customerTrappedMC");
    if (!model) {
        return nullptr;
    }
    std::string name = gameObj->obf_3A->getCustomerClipName(whichCharacter);
    if (style == 2) {
        name += "2";
    } else if (style == 3) {
        name += "3";
    }
    // Each part: model.<slot>.addChild(new customer_<name>_<part>()). A missing optional part is skipped (the
    // original's try/catch); a missing required one throws there, and is skipped here too.
    auto attach = [&](const char *slot, const std::string &cls) {
        engine::MovieClip *holder = model->findChild(slot);
        std::unique_ptr<engine::MovieClip> part = engine::MovieClip::create(symbols, cls);
        if (holder && part) {
            holder->addChild(std::move(part), "clip");
        }
    };
    const std::string prefix = "customer_" + name + "_";
    attach("body", prefix + "body");
    attach("head", prefix + "head");
    attach("eyes", prefix + "eyes");
    attach("mouth", prefix + "mouth");
    attach("neck", prefix + "neck");
    attach("hair", prefix + "hair");
    attach("back_hair", prefix + "back_hair");
    attach("front_shoe", prefix + "foot");
    attach("back_shoe", prefix + "foot");
    attach("fronthand", prefix + "hand");
    attach("backhand", prefix + "hand2");
    attach("front_upperarm", prefix + "upperarm");
    attach("back_upperarm", prefix + "upperarm");
    attach("front_forearm", prefix + "forearm");
    attach("back_forearm", prefix + "forearm");
    attach("cross_backforearm", prefix + "forearm");
    attach("cross_back_hand", prefix + "hand2");
    const std::string weapon =
        "weapon_" + gameObj->obf_3A->getWeaponClipName(gameObj->obf_51->selectedCharacter, gameObj->obf_51->selectedStyle);
    attach("weapon", weapon);
    attach("cross_back_weap", weapon);
    model->setScale(1, 1);
    return model;
}

} // namespace papaGame::screens
