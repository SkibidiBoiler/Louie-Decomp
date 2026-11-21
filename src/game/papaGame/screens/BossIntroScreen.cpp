#include "game/papaGame/screens/BossIntroScreen.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::screens {

// AS: BossIntroScreen.BossIntroScreen
BossIntroScreen::BossIntroScreen(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupScreen();
}

// AS: BossIntroScreen.setupScreen — the boss and Papa Louie get a thin black glow in the original (GlowFilter);
// the port's MovieClip has no filters.
void BossIntroScreen::setupScreen()
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    clip = &gameObj->obf_5h->addChild(engine::MovieClip::create(symbols, "bossIntroMC"), "");
    engine::MovieClip *holder = clip->findChild("holder");
    std::unique_ptr<engine::MovieClip> papa = buildModel(gameObj->obf_3A->getCustomerIndex("Papa Louie"), 1);
    if (papa && holder) {
        papa->gotoAndStop(1);
        papa->gotoLabel("stand", true);
        papa->setScale(0.27f, 0.27f);
        papa->setPosition(525, 197);
        papaMC = &holder->addChild(std::move(papa), "");
    }
    if (auto mg = engine::MovieClip::create(symbols, "tilesheet4_mg")) {
        mgMC = &clip->addChildAtBottom(std::move(mg), "");
        mgMC->setY(-150);
    }
    if (auto bg = engine::MovieClip::create(symbols, "tilesheet4_bg")) {
        bgMC = &clip->addChildAtBottom(std::move(bg), "");
    }
    if (auto boss = engine::MovieClip::create(symbols, "boss_intro_radish"); boss && holder) {
        boss->setPosition(332, 160);
        bossMC = &holder->addChild(std::move(boss), "");
    }
    if (engine::MovieClip *iris = clip->findChild("iris")) {
        iris->gotoLabel("irisin", true);
    }
    gameObj->obf_7C->endMusic();
    gameObj->obf_7C->playTrack("MidBossTrack", 2147483647, 0, "outin");
}

// AS: BossIntroScreen.updateScreen (ENTER_FRAME) — at 120 frames the iris closes; on its frame 21 the level starts.
void BossIntroScreen::updateScreen()
{
    engine::MovieClip *iris = clip->findChild("iris");
    ++animTimer;
    if (animTimer == animTimerMax) {
        if (iris) {
            iris->gotoLabel("irisout", true);
        }
    } else if (iris && iris->currentFrame() + 1 == 21) {
        gameObj->obf_52();
        gameObj->obf_28(); // destroys this screen
    }
}

// AS: BossIntroScreen.destroy
void BossIntroScreen::destroy()
{
    if (bgMC) {
        clip->removeChild(bgMC);
    }
    if (mgMC) {
        clip->removeChild(mgMC);
    }
    bgMC = nullptr;
    mgMC = nullptr;
    engine::MovieClip *holder = clip->findChild("holder");
    if (bossMC) {
        if (holder) {
            holder->removeChild(bossMC);
        }
        bossMC = nullptr;
    }
    if (papaMC) {
        cleanupModel(papaMC);
        if (holder) {
            holder->removeChild(papaMC);
        }
        papaMC = nullptr;
    }
    gameObj->obf_5h->removeChild(clip);
    clip = nullptr;
}

// AS: BossIntroScreen.buildModel — the trapped puppet dressed as the customer (style 2 / 3 picks those outfits).
// QUIRK: the weapon is the selected character's, not the modelled customer's.
std::unique_ptr<engine::MovieClip> BossIntroScreen::buildModel(double whichCustomer, double whichStyle)
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    std::unique_ptr<engine::MovieClip> model = engine::MovieClip::create(symbols, "customerTrappedMC");
    if (!model) {
        return model;
    }
    std::string name = gameObj->obf_3A->getCustomerClipName(whichCustomer);
    if (whichStyle == 2) {
        name += "2";
    } else if (whichStyle == 3) {
        name += "3";
    }
    // A missing optional part is skipped (the original's try/catch); a missing required one throws there, and is
    // skipped here too.
    auto attach = [&](const char *slot, const std::string &cls) {
        engine::MovieClip *s = model->findChild(slot);
        std::unique_ptr<engine::MovieClip> part = engine::MovieClip::create(symbols, cls);
        if (s && part) {
            s->addChild(std::move(part), "clip");
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
    const std::string weapon = "weapon_" + gameObj->obf_3A->getWeaponClipName(gameObj->obf_51->selectedCharacter,
                                                                               gameObj->obf_51->selectedStyle);
    attach("weapon", weapon);
    attach("cross_back_weap", weapon);
    model->setScale(1, 1);
    return model;
}

// AS: BossIntroScreen.cleanupModel — the parts go with the puppet when it is removed; only the stop is kept.
void BossIntroScreen::cleanupModel(engine::MovieClip *whichModel)
{
    whichModel->stop();
}

} // namespace papaGame::screens
