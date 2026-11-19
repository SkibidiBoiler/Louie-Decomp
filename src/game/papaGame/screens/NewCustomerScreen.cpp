#include "game/papaGame/screens/NewCustomerScreen.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::screens {

using data::CustomerData;

// AS: NewCustomerScreen.NewCustomerScreen
NewCustomerScreen::NewCustomerScreen(obf_9G::obf_4R *gameObj_, double whichCustomer_)
    : gameObj(gameObj_), whichCustomer(whichCustomer_)
{
    setupScreen();
}

// AS: NewCustomerScreen.setupScreen
void NewCustomerScreen::setupScreen()
{
    gameObj->obf_7C->endMusic();
    gameObj->obf_7C->playSound("customer_overjoyed.wav");
    clip = &gameObj->obf_5h->addChild(engine::MovieClip::create(*gameObj->symbols, "newCustomerScreenMC"), "");
    buildModel();
    const data::CustomerDataFile *customer = gameObj->obf_3A->getCustomerData(whichCustomer);
    const std::string skill = customer ? customer->skillType : CustomerData::SKILL_NONE;
    if (engine::MovieClip *skillbox = clip->findChild("skillbox")) {
        if (skill != CustomerData::SKILL_NONE) {
            skillbox->setVisible(true);
            skillbox->gotoLabel(skill, false);
        } else {
            skillbox->setVisible(false);
            skillbox->stop();
        }
    }
    clip->setText("name_txt", gameObj->obf_3A->getCustomerName(whichCustomer));
}

// AS: NewCustomerScreen.updateScreen (ENTER_FRAME)
void NewCustomerScreen::updateScreen()
{
    if (clip->currentFrame() + 1 == clip->frameCount()) {
        MainMenuParams params;
        params.section = "map";
        gameObj->showApiScreen("MainMenu", params);
        gameObj->obf_6s(); // destroys this screen
    }
}

// AS: NewCustomerScreen.destroy
void NewCustomerScreen::destroy()
{
    if (model) {
        if (engine::MovieClip *holder = clip->findChild("charholder")) {
            holder->removeChild(model);
        }
        model = nullptr;
    }
    gameObj->obf_5h->removeChild(clip);
    clip = nullptr;
}

// AS: NewCustomerScreen.buildModel — the customer's puppet for their weapon type, dressed in outfit A, running.
void NewCustomerScreen::buildModel()
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    engine::MovieClip *holder = clip->findChild("charholder");
    const std::string type = gameObj->obf_3A->getCustomerType(whichCustomer);
    const char *puppet = type == CustomerData::WEAPON_SWING2    ? "customerTwoSwingMC"
                         : type == CustomerData::WEAPON_SCOOTER ? "customerScooterMC"
                         : type == CustomerData::WEAPON_KAHUNA  ? "customerKahunaMC"
                         : type == CustomerData::WEAPON_WHIP    ? "customerWhipMC"
                         : type == CustomerData::WEAPON_MELEE   ? "customerMeleeMC"
                         : type == CustomerData::WEAPON_TOSS    ? "customerTossMC"
                         : type == CustomerData::WEAPON_LONGGUN ? "customerLongGunMC"
                         : type == CustomerData::WEAPON_PISTOL  ? "customerPistolMC"
                         : type == CustomerData::WEAPON_BAZOOKA ? "customerBazookaMC"
                                                                : "customerOneSwingMC";
    std::unique_ptr<engine::MovieClip> made = engine::MovieClip::create(symbols, puppet);
    if (!made || !holder) {
        return;
    }
    made->setScale(-0.6f, 0.6f);
    made->setPosition(400, 119);
    model = &holder->addChild(std::move(made), "");
    const std::string name = gameObj->obf_3A->getCustomerClipName(whichCustomer);
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
    const std::string weapon = "weapon_" + gameObj->obf_3A->getWeaponClipName(whichCustomer, 1);
    attach("weapon", weapon);
    attach("cross_back_weap", weapon);
    if (name == "Boomer") {
        attach("glider", "glider_" + name);
    } else if (engine::MovieClip *glider = model->findChild("glider")) {
        glider->addChild(engine::MovieClip::createEmpty(symbols), "clip");
    }
    model->gotoAndStop(1);
    model->gotoLabel("run", true);
    if (name == "Connor") {
        model->gotoLabel("runconnor", true);
    }
}

} // namespace papaGame::screens
