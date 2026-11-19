#include "game/papaGame/screens/StageOutroScreen.h"

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::screens {

// AS: StageOutroScreen.StageOutroScreen
StageOutroScreen::StageOutroScreen(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupScreen();
}

// AS: StageOutroScreen.setupScreen
void StageOutroScreen::setupScreen()
{
    data::UserData &user = *gameObj->obf_51;
    clip = &gameObj->obf_5h->addChild(engine::MovieClip::create(*gameObj->symbols, "stageOutroMC"), "");
    clip->setPosition(350, 208);
    engine::MovieClip *inside = clip->findChild("inside");
    if (std::unique_ptr<engine::MovieClip> model = buildModel(user.selectedCharacter, user.selectedStyle)) {
        model->gotoAndStop(1);
        model->gotoLabel("fall", false);
        if (inside) {
            playerMC = &inside->addChild(std::move(model), "");
        }
    }
    if (user.savedCharacterIndex > 0 && !user.savedCharacterWasUnlocked) {
        if (std::unique_ptr<engine::MovieClip> model = buildModel(user.savedCharacterIndex, 1)) {
            model->gotoAndStop(1);
            model->gotoLabel("fall", false);
            model->setRotation(180);
            model->setPosition(-640, 100);
            if (inside) {
                savedMC = &inside->addChild(std::move(model), "");
            }
        }
    }
    if (!inside) {
        return;
    }
    for (int i = 1; i <= 3; ++i) {
        if (engine::MovieClip *coin = inside->findChild("warpcoin" + std::to_string(i))) {
            coin->setVisible(user.warpCoinsEarned >= i);
        }
    }
    for (int i = 8; i >= 1; --i) {
        if (engine::MovieClip *coin = inside->findChild("coin" + std::to_string(i))) {
            coin->setVisible(user.money >= i * 10);
        }
    }
    if (user.money > 0) {
        if (engine::MovieClip *coin = inside->findChild("coin1")) {
            coin->setVisible(true);
        }
    }
}

// AS: StageOutroScreen.updateScreen (ENTER_FRAME)
void StageOutroScreen::updateScreen()
{
    if (clip->currentFrame() + 1 == clip->frameCount()) {
        if (gameObj->obf_AQ->currentLevel == 8) {
            gameObj->obf_14();
        } else {
            gameObj->showApiScreen("EndOfDay");
        }
        gameObj->obf_58(); // destroys this screen
    }
}

// AS: StageOutroScreen.destroy
void StageOutroScreen::destroy()
{
    engine::MovieClip *inside = clip->findChild("inside");
    if (playerMC) {
        if (inside) {
            inside->removeChild(playerMC);
        }
        playerMC = nullptr;
    }
    if (savedMC) {
        if (inside) {
            inside->removeChild(savedMC);
        }
        savedMC = nullptr;
    }
    gameObj->obf_5h->removeChild(clip);
    clip = nullptr;
}

// AS: StageOutroScreen.buildModel — the customer puppet (customerTrappedMC) dressed with the customer's parts and
// the player's weapon. SKELETON: the black GlowFilter outline (filters are not drawn yet).
std::unique_ptr<engine::MovieClip> StageOutroScreen::buildModel(double whichCharacter, double style)
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
    // A missing optional part is skipped (the original's try/catch); a missing required one throws there, and
    // is skipped here too.
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
    // QUIRK: the rescued customer also holds the player's weapon (the weapon is always the selected customer's).
    const std::string weapon = "weapon_" + gameObj->obf_3A->getWeaponClipName(gameObj->obf_51->selectedCharacter,
                                                                              gameObj->obf_51->selectedStyle);
    attach("weapon", weapon);
    attach("cross_back_weap", weapon);
    model->setScale(1, 1);
    return model;
}

} // namespace papaGame::screens
