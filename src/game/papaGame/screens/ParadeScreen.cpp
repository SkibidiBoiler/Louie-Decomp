#include "game/papaGame/screens/ParadeScreen.h"

#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::screens {

using data::CustomerData;

namespace {

// Port: `<slot>.addChild(new <cls>())` for a puppet part; a missing part is skipped (the original throws for the
// required ones and catches the optional ones).
void attachPart(const engine::SymbolLibrary &symbols, engine::MovieClip &model, const char *slot,
                const std::string &cls)
{
    engine::MovieClip *s = model.findChild(slot);
    std::unique_ptr<engine::MovieClip> part = engine::MovieClip::create(symbols, cls);
    if (s && part) {
        s->addChild(std::move(part), "clip");
    }
}

// Port: DisplayObject.width of a clip added with addChild (its scale counts, its sign does not).
float clipWidth(const engine::MovieClip &c, float scaleX, float scaleY)
{
    float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
    engine::Affine m;
    m.a = scaleX;
    m.d = scaleY;
    c.accumulateBounds(m, minX, minY, maxX, maxY);
    return maxX > minX ? maxX - minX : 0.0f;
}

} // namespace

// AS: ParadeScreen.ParadeScreen
ParadeScreen::ParadeScreen(obf_9G::obf_4R *gameObj_, bool showWeapons_) : gameObj(gameObj_), showWeapons(showWeapons_)
{
    setupScreen();
}

// AS: ParadeScreen.setupScreen
void ParadeScreen::setupScreen()
{
    clip = &gameObj->obf_5h->addChild(engine::MovieClip::create(*gameObj->symbols, "paradeScreenMC"), "");
    setupCustomerGroups();
    releaseNextGroup();
    didStart = true;
    gameObj->obf_7C->playTrack("TitleTrack", 1, 0, "crossfade");
    if (engine::MovieClip *iris = clip->findChild("iris")) {
        iris->gotoLabel("irisin", true);
    }
}

// AS: ParadeScreen.updateScreen (ENTER_FRAME) — everyone moves right; one who is off the right edge goes (the iris
// closes after the last); the newest marcher lets the next one out once it is far enough in. The original's
// "CLOSE" / "FAR" debug traces are dropped.
void ParadeScreen::updateScreen()
{
    if (didStart) {
        engine::MovieClip *holder = clip->findChild("holder");
        for (int i = static_cast<int>(movingParts.size()) - 1; i >= 0; --i) {
            engine::MovieClip *part = movingParts[static_cast<size_t>(i)];
            const std::string &type = movingPartsType[static_cast<size_t>(i)];
            part->setX(part->x() + static_cast<float>(showWeapons ? runSpeed : walkSpeed));
            const float width = clipWidth(*part, static_cast<float>(customerScale), static_cast<float>(customerScale));
            if ((type == "customer" || type == "customermain") && part->x() - width / 2 > 720) {
                if (showWeapons) {
                    cleanupModel(part);
                } else {
                    clearCustomer(part);
                }
                if (holder) {
                    holder->removeChild(part);
                }
                movingParts.erase(movingParts.begin() + i);
                movingPartsType.erase(movingPartsType.begin() + i);
                if (movingParts.empty()) {
                    if (engine::MovieClip *iris = clip->findChild("iris")) {
                        iris->gotoLabel("irisout", true);
                    }
                }
            } else {
                const size_t order = static_cast<size_t>(whichOrder);
                if (showWeapons && type == "customermain" && part->x() > 80 &&
                    whichOrder < static_cast<double>(allCustomerSkills.size()) - 1 &&
                    allCustomerSkills[order + 1] == allCustomerSkills[order]) {
                    releaseNextGroup();
                    movingPartsType[static_cast<size_t>(i)] = "customer";
                    continue;
                }
                if (showWeapons && type == "customermain" && part->x() > 200) {
                    releaseNextGroup();
                    movingPartsType[static_cast<size_t>(i)] = "customer";
                    continue;
                }
                if (!showWeapons && type == "customermain" && part->x() > 100) {
                    releaseNextGroup();
                    movingPartsType[static_cast<size_t>(i)] = "customer";
                }
            }
        }
        engine::MovieClip *iris = clip->findChild("iris");
        if (iris && iris->currentFrame() + 1 == 21) {
            gameObj->obf_49();
            gameObj->obf_3q(); // destroys this screen
        }
    }
}

// AS: ParadeScreen.destroy
void ParadeScreen::destroy()
{
    gameObj->obf_5h->removeChild(clip);
    clip = nullptr;
}

// AS: ParadeScreen.releaseNextGroup — the next unlocked customer starts at the left edge.
// QUIRK: the group size the original tests (to stagger a group's members up and down) is a local that is never set
// (NaN), so every marcher is added on top, unstaggered; that branch is kept out.
void ParadeScreen::releaseNextGroup()
{
    ++whichOrder;
    if (whichOrder < static_cast<double>(allCustomerIDs.size())) {
        const double id = allCustomerIDs[static_cast<size_t>(whichOrder)];
        std::unique_ptr<engine::MovieClip> marcher = showWeapons ? buildModel(id) : buildCustomer(id);
        if (!marcher) {
            return;
        }
        marcher->setScale(static_cast<float>(customerScale * -1), static_cast<float>(customerScale));
        marcher->setPosition(static_cast<float>(startX), static_cast<float>(customerStartY));
        const double r1 = as3::random();
        const double r2 = as3::random();
        if (!showWeapons) {
            if (r1 > 0.7) {
                marcher->gotoLabel(r2 > 0.8 ? "walkwave2" : r2 > 0.4 ? "walkwave2_2" : "walkwave2_3", true);
            } else if (r1 > 0.4) {
                marcher->gotoLabel(r2 > 0.8 ? "walkwave" : r2 > 0.5 ? "walkwave_2" : "walkwave_3", true);
            } else {
                marcher->gotoLabel(r2 > 0.7 ? "walk" : r2 > 0.4 ? "walk_2" : "walk_3", true);
            }
        }
        engine::MovieClip *holder = clip->findChild("holder");
        if (!holder) {
            return;
        }
        movingParts.push_back(&holder->addChild(std::move(marcher), ""));
        movingPartsType.push_back("customermain");
    }
}

// AS: ParadeScreen.setupCustomerGroups — the unlocked customers in roster order, with their skills.
void ParadeScreen::setupCustomerGroups()
{
    const data::UserData &user = *gameObj->obf_51;
    allCustomerIDs.clear();
    allCustomerSkills.clear();
    for (size_t i = 0; i < user.customersUnlocked.size(); ++i) {
        if (user.customersUnlocked[i] == 1) {
            allCustomerIDs.push_back(static_cast<double>(i));
            allCustomerSkills.push_back(gameObj->obf_3A->getCustomerSkill(static_cast<double>(i)));
        }
    }
}

// AS: ParadeScreen.getCustomerClipName
std::string ParadeScreen::getCustomerClipName(double index) const
{
    return gameObj->obf_3A->getCustomerClipName(index);
}

// AS: ParadeScreen.buildCustomer — the walking parade puppet dressed as the customer.
std::unique_ptr<engine::MovieClip> ParadeScreen::buildCustomer(double index)
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    std::unique_ptr<engine::MovieClip> model = engine::MovieClip::create(symbols, "customerParadeMC");
    if (!model) {
        return model;
    }
    const std::string prefix = "customer_" + getCustomerClipName(index) + "_";
    attachPart(symbols, *model, "body", prefix + "body");
    attachPart(symbols, *model, "head", prefix + "head");
    attachPart(symbols, *model, "eyes", prefix + "eyes");
    attachPart(symbols, *model, "mouth", prefix + "mouth");
    attachPart(symbols, *model, "neck", prefix + "neck");
    attachPart(symbols, *model, "hair", prefix + "hair");
    attachPart(symbols, *model, "back_hair", prefix + "back_hair");
    attachPart(symbols, *model, "front_shoe", prefix + "foot");
    attachPart(symbols, *model, "back_shoe", prefix + "foot");
    attachPart(symbols, *model, "fronthand", prefix + "hand");
    attachPart(symbols, *model, "backhand", prefix + "hand2");
    attachPart(symbols, *model, "front_upperarm", prefix + "upperarm");
    attachPart(symbols, *model, "back_upperarm", prefix + "upperarm");
    attachPart(symbols, *model, "front_forearm", prefix + "forearm");
    attachPart(symbols, *model, "back_forearm", prefix + "forearm");
    return model;
}

// AS: ParadeScreen.clearCustomer — the parts go with the puppet when it is removed; only the stop is kept.
void ParadeScreen::clearCustomer(engine::MovieClip *customer)
{
    customer->stop();
}

// AS: ParadeScreen.buildModel — the customer's running puppet for their weapon type, with weapon (and Boomer's
// glider).
std::unique_ptr<engine::MovieClip> ParadeScreen::buildModel(double index)
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    const std::string type = gameObj->obf_3A->getCustomerType(index);
    const char *puppet = type == CustomerData::WEAPON_SWING1    ? "customerOneSwingMC"
                         : type == CustomerData::WEAPON_SWING2  ? "customerTwoSwingMC"
                         : type == CustomerData::WEAPON_SCOOTER ? "customerScooterMC"
                         : type == CustomerData::WEAPON_KAHUNA  ? "customerKahunaMC"
                         : type == CustomerData::WEAPON_WHIP    ? "customerWhipMC"
                         : type == CustomerData::WEAPON_MELEE   ? "customerMeleeMC"
                         : type == CustomerData::WEAPON_TOSS    ? "customerTossMC"
                         : type == CustomerData::WEAPON_LONGGUN ? "customerLongGunMC"
                         : type == CustomerData::WEAPON_PISTOL  ? "customerPistolMC"
                         : type == CustomerData::WEAPON_BAZOOKA ? "customerBazookaMC"
                                                                : "customerOneSwingMC";
    std::unique_ptr<engine::MovieClip> model = engine::MovieClip::create(symbols, puppet);
    if (!model) {
        return model;
    }
    const std::string name = gameObj->obf_3A->getCustomerClipName(index);
    const std::string prefix = "customer_" + name + "_";
    attachPart(symbols, *model, "body", prefix + "body");
    attachPart(symbols, *model, "head", prefix + "head");
    attachPart(symbols, *model, "eyes", prefix + "eyes");
    attachPart(symbols, *model, "mouth", prefix + "mouth");
    attachPart(symbols, *model, "neck", prefix + "neck");
    attachPart(symbols, *model, "hair", prefix + "hair");
    attachPart(symbols, *model, "back_hair", prefix + "back_hair");
    attachPart(symbols, *model, "front_shoe", prefix + "foot");
    attachPart(symbols, *model, "back_shoe", prefix + "foot");
    attachPart(symbols, *model, "fronthand", prefix + "hand");
    attachPart(symbols, *model, "backhand", prefix + "hand2");
    attachPart(symbols, *model, "front_upperarm", prefix + "upperarm");
    attachPart(symbols, *model, "back_upperarm", prefix + "upperarm");
    attachPart(symbols, *model, "front_forearm", prefix + "forearm");
    attachPart(symbols, *model, "back_forearm", prefix + "forearm");
    attachPart(symbols, *model, "cross_backforearm", prefix + "forearm");
    attachPart(symbols, *model, "cross_back_hand", prefix + "hand2");
    const std::string weapon = "weapon_" + gameObj->obf_3A->getWeaponClipName(index, 1);
    attachPart(symbols, *model, "weapon", weapon);
    attachPart(symbols, *model, "cross_back_weap", weapon);
    if (name == "Boomer") {
        attachPart(symbols, *model, "glider", "glider_" + name);
    } else if (engine::MovieClip *glider = model->findChild("glider")) {
        glider->addChild(engine::MovieClip::createEmpty(symbols), "");
    }
    model->gotoAndStop(1);
    model->gotoLabel("run", true);
    if (name == "Connor") {
        model->gotoLabel("runconnor", true);
    }
    model->mouseChildren = false;
    return model;
}

// AS: ParadeScreen.cleanupModel — the parts go with the puppet when it is removed; only the rewind is kept.
void ParadeScreen::cleanupModel(engine::MovieClip *model)
{
    model->gotoAndStop(1);
}

} // namespace papaGame::screens
