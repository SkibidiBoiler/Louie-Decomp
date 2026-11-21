#include "game/papaGame/screens/GameOutroScreen.h"

#include "game/obf_9G/obf_4R.h"

namespace papaGame::screens {

// AS: GameOutroScreen.GameOutroScreen
GameOutroScreen::GameOutroScreen(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupScreen();
}

// AS: GameOutroScreen.setupScreen
void GameOutroScreen::setupScreen()
{
    clip = &gameObj->obf_5h->addChild(engine::MovieClip::create(*gameObj->symbols, "gameEndingMC"), "");
    setupModels();
    if (engine::MovieClip *iris = clip->findChild("iris")) {
        iris->gotoLabel("irisin", true);
    }
}

// AS: GameOutroScreen.updateScreen (ENTER_FRAME)
void GameOutroScreen::updateScreen()
{
    engine::MovieClip *iris = clip->findChild("iris");
    ++outroTimer;
    if (outroTimer == outroTimerMax) {
        if (iris) {
            iris->gotoLabel("irisout", true);
        }
    } else if (outroTimer > outroTimerMax) {
        if (iris && iris->currentFrame() + 1 == 21) {
            finishGameOutro();
        }
    }
}

// AS: GameOutroScreen.finishGameOutro — the original opens the API's EndOfDay screen, then removes this one.
void GameOutroScreen::finishGameOutro()
{
    gameObj->showApiScreen("EndOfDay");
    gameObj->obf_2r(); // destroys this screen
}

// AS: GameOutroScreen.destroy
void GameOutroScreen::destroy()
{
    for (engine::MovieClip *m : models) {
        clearCustomer(m);
    }
    models.clear(); // the row holders own them; they go with the clip
    gameObj->obf_5h->removeChild(clip);
    clip = nullptr;
}

// AS: GameOutroScreen.setupModels — the unlocked customers of each row, posed (some with their own pose); the
// right-hand ones face left and go under the others.
void GameOutroScreen::setupModels()
{
    setupRow(frontRowNames, frontRowX, frontRowY, frontRowScale, "front_holder", 0);
    setupRow(middleRowNames, middleRowX, middleRowY, middleRowScale, "middle_holder", 1);
    setupRow(backRowNames, backRowX, backRowY, backRowScale, "back_holder", 2);
    for (const char *h : {"front_holder", "middle_holder", "back_holder"}) {
        if (engine::MovieClip *holder = clip->findChild(h)) {
            holder->mouseChildren = false;
        }
    }
}

void GameOutroScreen::setupRow(const std::vector<std::string> &names, const std::vector<double> &xs,
                               const std::vector<double> &ys, double scale, const char *holderName, int row)
{
    const data::UserData &user = *gameObj->obf_51;
    engine::MovieClip *holder = clip->findChild(holderName);
    for (size_t i = 0; i < names.size(); ++i) {
        const double index = gameObj->obf_3A->getCustomerIndex(names[i]);
        if (index <= -1 || !user.hasCustomerUnlocked(index)) {
            continue;
        }
        std::unique_ptr<engine::MovieClip> m = buildCustomer(index);
        if (!m || !holder) {
            continue;
        }
        const bool flipped = row == 0 ? (i >= 3 && i != 6) : row == 1 ? i >= 5 : i >= 6;
        m->setScale(static_cast<float>(flipped ? scale * -1 : scale), static_cast<float>(scale));
        m->setPosition(static_cast<float>(xs[i]), static_cast<float>(ys[i]));
        m->gotoLabel("normal", false);
        const std::string &n = names[i];
        if (row == 0) {
            if (n == "Papa Louie") {
                m->gotoLabel("papa", false);
            } else if (n == "Penny") {
                m->gotoLabel("frontthumb", false);
            } else if (n == "Greg") {
                m->gotoLabel("smile", false);
            }
        } else if (row == 1) {
            if (n == "Prudence") {
                m->gotoLabel("yes", false);
            } else if (n == "Taylor" || n == "Mindy") {
                m->gotoLabel("smile", false);
            }
        } else {
            if (n == "Connor") {
                m->gotoLabel("yes", false);
            } else if (n == "Zoe" || n == "James") {
                m->gotoLabel("smile", false);
            } else if (n == "Akari") {
                m->gotoLabel("backthumb", false);
            }
        }
        const bool atBottom = flipped || (row == 0 && i == 6);
        models.push_back(atBottom ? &holder->addChildAtBottom(std::move(m), "") : &holder->addChild(std::move(m), ""));
    }
}

// AS: GameOutroScreen.buildCustomer — the posing puppet dressed as the customer.
std::unique_ptr<engine::MovieClip> GameOutroScreen::buildCustomer(double index)
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    std::unique_ptr<engine::MovieClip> model = engine::MovieClip::create(symbols, "customerPoseMC");
    if (!model) {
        return model;
    }
    auto attach = [&](const char *slot, const std::string &cls) {
        engine::MovieClip *s = model->findChild(slot);
        std::unique_ptr<engine::MovieClip> part = engine::MovieClip::create(symbols, cls);
        if (s && part) {
            s->addChild(std::move(part), "clip");
        }
    };
    const std::string prefix = "customer_" + gameObj->obf_3A->getCustomerClipName(index) + "_";
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
    return model;
}

// AS: GameOutroScreen.clearCustomer — the parts go with the puppet when it is removed; only the stop is kept.
void GameOutroScreen::clearCustomer(engine::MovieClip *customer)
{
    customer->stop();
}

} // namespace papaGame::screens
