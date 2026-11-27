#include "game/papaGame/screens/MainMenuScreen.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cctype>
#include <cmath>
#include <iterator>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_81.h"

namespace papaGame::screens {

namespace {

using data::CustomerData;

} // namespace

// AS: MainMenuScreen.MainMenuScreen
MainMenuScreen::MainMenuScreen(obf_9G::obf_4R *gameObj_, engine::MovieClip *container_, const MainMenuParams &params_)
    : gameObj(gameObj_), container(container_), params(params_)
{
    setupScreen();
}

// AS: MainMenuScreen.setupScreen — the API events the bars send are wired through the game's container.
void MainMenuScreen::setupScreen()
{
    if (params.useLevel >= 0) {
        whichLevel = params.useLevel;
    }
    isOnCharSelect = params.isCharSelect;
    clip = &container->addChild(engine::MovieClip::create(*gameObj->symbols, "mainMenuMC"), "");
    gameObj->addContainerListener("clickCredits", [this] { clickCredits(); });
    gameObj->addContainerListener("clickMedals", [this] { clickMedals(); });
    gameObj->addContainerListener("clickBaddies", [this] { clickBaddies(); });
    gameObj->addContainerListener("clickControls", [this] { clickControls(); });
    gameObj->addContainerListener("clickHelp", [this] { clickHelp(); });
    gameObj->addContainerListener("clickExit", [this] { clickExit(); });
    gameObj->addContainerListener("clickCreditsRedirect", [this] { setSection("credits"); });
    gameObj->addContainerListener("clickCreditsRedirectBack", [this] { setSection("map"); });
    gameObj->addContainerListener("clickMap", [this] { clickMap(); });
    gameObj->addContainerListener("clickStartLevel", [this] { clickStartLevel(); });
    gameObj->addContainerListener("clickBackToMap", [this] { clickBackToMap(); });
    gameObj->addContainerListener("clickExitMapSelect", [this] { clickExitMapSelect(); });
    gameObj->addContainerListener("clickInfo", [this] { clickInfo(); });
    gameObj->addContainerListener("clickBackToGame", [this] { clickBackToGame(); });
    gameObj->addContainerListener("clickQuit", [this] { clickQuit(); });
    setupMap();
    if (isOnCharSelect) {
        setupCharacter();
    }
    if (!isOnCharSelect) {
        setupInfo();
    }
    setupBaddies();
    setupControls();
    setupCredits();
    setupHelp();
    setupMedals();
    setupConfirmQuit();
    if (gameObj->obf_AQ->currentScreenData == nullptr && gameObj->obf_7C->currentTrack != "otherscreens.wav") {
        gameObj->obf_7C->playTrack("AlternateTrack", 1, 0, "outin");
    }
    setSection(params.section.empty() ? "map" : params.section);
}

// AS: MainMenuScreen.setupCharacter — the area's challenges, the customers' thumbnails and the outfit buttons.
void MainMenuScreen::setupCharacter(bool setup)
{
    engine::MovieClip *character = section("character");
    if (!character) {
        return;
    }
    data::UserData &user = *gameObj->obf_51;
    if (!setup) {
        updatingChar_ = false;
        return;
    }
    if (engine::MovieClip *holder = character->findChild("charholder")) {
        holder->mouseEnabled = false;
        holder->mouseChildren = false;
    }
    character->setText("title_txt", gameObj->obf_AQ->getWorldTitle(whichLevel));
    character->setText("levelnum_txt", whichLevel == 9 ? "X" : as3::String(whichLevel + 1));
    character->setText("score_txt", obf_AF::obf_81::obf_7w(user.getLevelHighScore(whichLevel)));
    engine::MovieClip *levelinside = character->findChild("levelinside");
    if (levelinside) {
        if (engine::MovieClip *inside = levelinside->findChild("inside")) {
            inside->gotoAndStop(static_cast<int>(whichLevel) + 1);
        }
        levelinside->setMask(character->findChild("levelmask"));
    }
    for (const char *link : {"link_btn", "underline"}) {
        if (engine::MovieClip *c = character->findChild(link)) {
            c->setVisible(false); // the customers' "first game" web links
        }
    }
    for (int i = 0; i < static_cast<int>(numCharacters); ++i) {
        engine::MovieClip *thumb = character->findChild("thumb" + std::to_string(i));
        if (!thumb) {
            continue;
        }
        for (const char *part : {"hilite", "roll", "newbanner"}) {
            if (engine::MovieClip *c = thumb->findChild(part)) {
                c->setVisible(false);
            }
        }
        if (i < static_cast<int>(user.customersUsed.size()) && user.customersUsed[static_cast<size_t>(i)] == 0) {
            if (engine::MovieClip *banner = thumb->findChild("newbanner")) {
                banner->setVisible(true);
            }
        }
        const double index = i;
        thumb->onMouseDown = [this, index] { clickCharacterThumb(index); };
        // AS: MainMenuScreen.rolloverCharacterThumb
        thumb->onRollOver = [thumb] {
            if (engine::MovieClip *roll = thumb->findChild("roll")) {
                roll->setVisible(true);
            }
        };
        // AS: MainMenuScreen.rolloutCharacterThumb
        thumb->onRollOut = [thumb] {
            if (engine::MovieClip *roll = thumb->findChild("roll")) {
                roll->setVisible(false);
            }
        };
        if (const data::CustomerDataFile *c = gameObj->obf_3A->getCustomerData(i)) {
            thumb->gotoLabel(c->skillType, false);
        }
        if (engine::MovieClip *holder = thumb->findChild("holder")) {
            holder->addBitmap(gameObj->obf_3A->getCustomerBitmap(i));
        }
        if (!user.hasCustomerUnlocked(i)) {
            thumb->setVisible(false);
        }
    }
    SDL_Log("Setup Challenges for level: %s", as3::String(whichLevel).c_str());
    for (int i = 1; i <= 6; ++i) {
        engine::MovieClip *icon = character->findChild("icon" + std::to_string(i));
        engine::MovieClip *panel = character->findChild("panel" + std::to_string(i));
        if (!icon || !panel) {
            continue;
        }
        icon->gotoAndStop(user.hasCompletedChallenge(whichLevel, i) ? 2 : 1);
        icon->setText("num_txt", std::to_string(i));
        const std::string type = gameObj->obf_94->getChallengeType(whichLevel, i);
        const std::string target = gameObj->obf_94->getChallengeTargetAmount(whichLevel, i);
        const std::string skill = gameObj->obf_94->getChallengeSkillNeeded(whichLevel, i);
        engine::MovieClip *panelIcon = panel->findChild("icon");
        if (type == data::Challenge::RESCUE) {
            panel->setText("description_txt", gameObj->lang->get("challenge.rescue"));
            const double trapped = gameObj->obf_3A->getTrappedCustomerIndex(whichLevel, i);
            if (panelIcon) {
                panelIcon->gotoAndStop(1);
                engine::MovieClip::BitmapChild face = gameObj->obf_3A->getCustomerBitmap(trapped);
                face.x = -8;
                face.y = -8;
                const std::string name = gameObj->obf_3A->getCustomerName(trapped);
                if (name == "Georgito" || name == "Yippy" || name == "Greg") {
                    face.y -= 8;
                }
                if (engine::MovieClip *holder = panelIcon->findChild("holder")) {
                    holder->addBitmap(face);
                    holder->setMask(panelIcon->findChild("masker"));
                }
            }
        } else if (type == data::Challenge::BURGERZILLAS) {
            panel->setText("description_txt", gameObj->lang->get("challenge.defeat", target));
            if (panelIcon) {
                panelIcon->gotoAndStop(2);
            }
        } else if (type == data::Challenge::COINS) {
            panel->setText("description_txt", gameObj->lang->get("challenge.find", target));
            if (panelIcon) {
                panelIcon->gotoAndStop(3);
            }
        } else {
            panel->setText("description_txt", gameObj->lang->get("challenge.find", target));
            if (panelIcon) {
                panelIcon->gotoAndStop(4 + static_cast<int>(whichLevel));
            }
        }
        engine::MovieClip *skillClip = panel->findChild("skill");
        engine::MovieClip *needs = panel->findChild("needs");
        if (skill == CustomerData::SKILL_NONE || skill.empty()) {
            if (skillClip) {
                skillClip->stop();
                skillClip->setVisible(false);
            }
            if (needs) {
                needs->setVisible(false);
            }
        } else if (skillClip) {
            skillClip->gotoLabel(skill, false);
        }
        if (type.empty()) {
            panel->setVisible(false);
            icon->setVisible(false);
        }
    }
    const char *styles[] = {"styleA", "styleB", "styleC"};
    for (int s = 0; s < 3; ++s) {
        engine::MovieClip *button = character->findChild(styles[s]);
        if (!button) {
            continue;
        }
        const double style = s + 1;
        button->onMouseDown = [this, style] { clickCharacterStyle(style); };
        // AS: MainMenuScreen.rolloverCharacterStyle
        button->onRollOver = [button] {
            if (engine::MovieClip *r = button->findChild("rolloverclip")) {
                r->setVisible(true);
            }
        };
        // AS: MainMenuScreen.rolloutCharacterStyle
        button->onRollOut = [button] {
            if (engine::MovieClip *r = button->findChild("rolloverclip")) {
                r->setVisible(false);
            }
        };
        if (engine::MovieClip *r = button->findChild("rolloverclip")) {
            r->setVisible(false);
        }
    }
    selectCharacter(user.selectedCharacter, user.selectedStyle);
    for (const char *social : {"bonus_facebook_btn", "bonus_twitter_btn"}) {
        if (engine::MovieClip *b = character->findChild(social)) {
            b->setVisible(false); // the Facebook / Twitter bonus links
        }
    }
    character->setText("totalmoney_txt", obf_AF::obf_81::obf_7w(user.getTotalMoney()));
    updatingChar_ = true;
}

// AS: MainMenuScreen.updateCharacter (ENTER_FRAME) — the chosen customer runs in, the old one runs out.
void MainMenuScreen::updateCharacter()
{
    engine::MovieClip *character = section("character");
    engine::MovieClip *holder = character ? character->findChild("charholder") : nullptr;
    if (characterModel) {
        if (characterModel->x() > characterTargetX) {
            characterModel->setX(static_cast<float>(characterModel->x() - characterSpeed));
            if (characterModel->x() <= characterTargetX) {
                characterModel->setX(static_cast<float>(characterTargetX));
                characterModel->gotoLabel("stand", true);
                if (gameObj->obf_3A->getCustomerClipName(selectedCharacterIndex) == "Connor") {
                    characterModel->gotoLabel("standconnor", true);
                }
            }
        }
    }
    for (int i = static_cast<int>(leavingModels.size()) - 1; i >= 0; --i) {
        engine::MovieClip *leaving = leavingModels[static_cast<size_t>(i)];
        leaving->setX(static_cast<float>(leaving->x() - characterSpeed));
        if (leaving->x() <= characterLeaveX) {
            if (holder) {
                holder->removeChild(leaving);
            }
            leavingModels.erase(leavingModels.begin() + i);
        }
    }
    engine::MovieClip *iris = character ? character->findChild("iris") : nullptr;
    if (iris && iris->currentFrameLabel() == "stopirisout" && isClosing) {
        isClosing = false;
        closeMainMenuScreen();
    }
}

// AS: MainMenuScreen.selectCharacter — the customer's name, game, weapon and skill, and its puppet.
void MainMenuScreen::selectCharacter(double index, double style)
{
    if (isClosing) {
        return;
    }
    engine::MovieClip *character = section("character");
    lastCharacterIndex = selectedCharacterIndex;
    selectedCharacterIndex = index;
    selectedStyle = style;
    for (int i = 0; i < static_cast<int>(numCharacters); ++i) {
        if (engine::MovieClip *thumb = character->findChild("thumb" + std::to_string(i))) {
            if (engine::MovieClip *hilite = thumb->findChild("hilite")) {
                hilite->setVisible(i == static_cast<int>(index));
            }
        }
    }
    const data::CustomerDataFile *c = gameObj->obf_3A->getCustomerData(index);
    if (!c) {
        return;
    }
    character->setText("name_txt", c->customerName);
    character->setText("firstgame_txt", c->customerFirstGame);
    character->setText("weaponname_txt", c->weaponName == "Pizza Paddle" && style == 3 ? "Beach Umbrella" : c->weaponName);
    if (engine::MovieClip *icon = character->findChild("skillicon")) {
        icon->gotoLabel(c->skillType, false);
    }
    const std::string skill = c->skillType;
    const char *key = skill == CustomerData::SKILL_CRAWL        ? "skill.crawl"
                      : skill == CustomerData::SKILL_DOUBLEJUMP ? "skill.doublejump"
                      : skill == CustomerData::SKILL_WALLJUMP   ? "skill.walljump"
                      : skill == CustomerData::SKILL_GLIDE      ? "skill.glide"
                      : skill == CustomerData::SKILL_POUND      ? "skill.pound"
                      : skill == CustomerData::SKILL_PUSH       ? "skill.push"
                                                                : "skill.none";
    character->setText("skillname_txt", gameObj->lang->get(key));
    buildModel(lastCharacterName == c->customerName);
    lastCharacterName = c->customerName;
    gameObj->obf_51->selectedCharacter = selectedCharacterIndex;
    gameObj->obf_51->selectedStyle = selectedStyle;
    updateStyleButtons();
}

// AS: MainMenuScreen.updateStyleButtons — worn, bought, affordable (with its price) or locked.
void MainMenuScreen::updateStyleButtons()
{
    engine::MovieClip *character = section("character");
    data::UserData &user = *gameObj->obf_51;
    if (engine::MovieClip *a = character->findChild("styleA")) {
        a->gotoAndStop(selectedStyle == 1 ? 1 : 2);
    }
    const char *names[] = {"styleB", "styleC"};
    for (int s = 0; s < 2; ++s) {
        engine::MovieClip *button = character->findChild(names[s]);
        if (!button) {
            continue;
        }
        const double outfit = s + 1;
        const double price = gameObj->obf_AQ->getOutfitPrice(selectedCharacterIndex, outfit);
        if (selectedStyle == outfit + 1) {
            button->gotoAndStop(1);
            button->setText("price_txt", "");
        } else if (user.hasOutfitUnlocked(selectedCharacterIndex, outfit)) {
            button->gotoAndStop(2);
            button->setText("price_txt", "");
        } else if (user.getTotalMoney() >= price) {
            button->gotoAndStop(3);
            button->setText("price_txt", as3::String(price));
        } else {
            button->gotoAndStop(4);
            button->setText("price_txt", "");
        }
    }
}

// AS: MainMenuScreen.clickCharacterStyle — wear an outfit, buying it first when it is not owned.
void MainMenuScreen::clickCharacterStyle(double style)
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    if (isClosing) {
        return;
    }
    data::UserData &user = *gameObj->obf_51;
    bool bought = false, wear = false;
    if (!user.hasOutfitUnlocked(selectedCharacterIndex, style - 1)) {
        bought = user.purchaseOutfit(selectedCharacterIndex, style - 1);
        if (bought) {
            wear = true;
            gameObj->obf_64(false);
        }
    } else {
        wear = true;
    }
    if (!wear) {
        return;
    }
    selectedStyle = style;
    buildModel(true);
    user.selectedCharacter = selectedCharacterIndex;
    user.selectedStyle = selectedStyle;
    engine::MovieClip *character = section("character");
    if (gameObj->obf_3A->getCustomerName(selectedCharacterIndex) == "Papa Louie") {
        character->setText("weaponname_txt", selectedStyle == 3 ? "Beach Umbrella" : "Pizza Paddle");
    }
    if (bought) {
        gameObj->obf_7C->playSound("getstar.wav");
        if (engine::MovieClip *burst = character->findChild("starburst")) {
            burst->gotoAndPlay(2);
        }
        character->setText("totalmoney_txt", obf_AF::obf_81::obf_7w(user.getTotalMoney()));
    }
    updateStyleButtons();
}

// AS: MainMenuScreen.clickCharacterThumb — the customer in its best outfit.
void MainMenuScreen::clickCharacterThumb(double index)
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    gameObj->obf_c("character", false);
    selectCharacter(index, gameObj->obf_51->getBestOutfit(index));
}

// AS: MainMenuScreen.buildModel — a new customer runs in (the old one runs out), or the same one is re-dressed.
void MainMenuScreen::buildModel(bool sameCharacter)
{
    engine::MovieClip *character = section("character");
    engine::MovieClip *holder = character ? character->findChild("charholder") : nullptr;
    if (!holder) {
        return;
    }
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    const bool first = characterModel == nullptr;
    if (!sameCharacter && characterModel) {
        characterModel->gotoLabel("run", true);
        if (gameObj->obf_3A->getCustomerClipName(lastCharacterIndex) == "Connor") {
            characterModel->gotoLabel("runconnor", true);
        }
        leavingModels.push_back(characterModel);
        characterModel = nullptr;
    }
    if (!sameCharacter || !characterModel) {
        const std::string type = gameObj->obf_3A->getCustomerType(selectedCharacterIndex);
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
        std::unique_ptr<engine::MovieClip> model = engine::MovieClip::create(symbols, puppet);
        if (!model) {
            return;
        }
        model->setScale(0.57f, 0.57f);
        model->setPosition(static_cast<float>(first ? characterTargetX : characterStartX),
                           static_cast<float>(characterTargetY));
        characterModel = &holder->addChild(std::move(model), "");
    } else {
        // cleanupModel: the same puppet, re-dressed.
        const char *slots[] = {"body", "head", "eyes", "mouth", "neck", "hair", "back_hair", "front_shoe",
                               "back_shoe", "fronthand", "backhand", "front_upperarm", "back_upperarm",
                               "front_forearm", "back_forearm", "cross_backforearm", "cross_back_hand", "weapon",
                               "cross_back_weap", "glider"};
        for (const char *slot : slots) {
            if (engine::MovieClip *s = characterModel->findChild(slot)) {
                if (engine::MovieClip *part = s->firstChildClip()) {
                    s->removeChild(part);
                }
            }
        }
    }
    const std::string clipName = gameObj->obf_3A->getCustomerClipName(selectedCharacterIndex);
    std::string name = clipName;
    if (selectedStyle == 2) {
        name += "2";
    } else if (selectedStyle == 3) {
        name += "3";
    }
    auto attach = [&](const char *slot, const std::string &cls) {
        engine::MovieClip *s = characterModel->findChild(slot);
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
    const std::string weapon = "weapon_" + gameObj->obf_3A->getWeaponClipName(selectedCharacterIndex, selectedStyle);
    attach("weapon", weapon);
    attach("cross_back_weap", weapon);
    if (clipName == "Boomer" || gameObj->obf_3A->getCustomerData(selectedCharacterIndex)->gliderHasStyles) {
        attach("glider", "glider_" + clipName);
    } else if (engine::MovieClip *glider = characterModel->findChild("glider")) {
        glider->addChild(engine::MovieClip::createEmpty(symbols), "clip");
    }
    const bool connor = clipName == "Connor";
    const bool atTarget = characterModel->x() == characterTargetX;
    if (!first && !sameCharacter) {
        characterModel->gotoAndStop(1);
        characterModel->gotoLabel(connor ? "runconnor" : "run", true);
    } else if (first || atTarget) {
        characterModel->gotoAndStop(1);
        characterModel->gotoLabel(connor ? "standconnor" : "stand", true);
    } else {
        characterModel->gotoAndStop(1);
        characterModel->gotoLabel(connor ? "runconnor" : "run", true);
    }
    characterModel->mouseEnabled = false;
    characterModel->mouseChildren = false;
}

// AS: MainMenuScreen.setupMap — the unlocked areas, the totals, and the reveal of a newly opened area.
// The store upsell is not shown.
void MainMenuScreen::setupMap(bool setup)
{
    engine::MovieClip *map = section("map");
    if (!map || !setup) {
        return;
    }
    data::UserData &user = *gameObj->obf_51;
    user.unlockNextLevel();
    if (engine::MovieClip *upsell = map->findChild("upsellMC")) {
        upsell->setVisible(false);
    }
    map->setText("score_txt", obf_AF::obf_81::obf_7w(user.getTotalScore()) + " PTS");
    map->setText("coins_txt", obf_AF::obf_81::obf_7w(user.getTotalMoney()));
    map->setText("warpcoins_txt", as3::String(user.getWarpCoins()));
    if (engine::MovieClip *bubble = map->findChild("rollover_bubble")) {
        bubble->setVisible(false);
        bubble->mouseEnabled = false;
        bubble->mouseChildren = false;
    }
    double last = -1;
    for (int i = 0; i < 10; ++i) {
        engine::MovieClip *world = map->findChild("world" + std::to_string(i + 1));
        if (!world) {
            continue;
        }
        const bool open = i < static_cast<int>(user.areasUnlocked.size()) && user.areasUnlocked[static_cast<size_t>(i)] == 1 &&
                          (i == 0 || user.getLevelHighScore(i - 1) > 0);
        if (open) {
            last = i;
            world->setVisible(true);
            world->setText("num_txt", i == 9 ? "X" : std::to_string(i + 1));
            if (engine::MovieClip *btn = world->findChild("btn")) {
                btn->onClick = [this, i] { clickPlayMap(i); };
                btn->onRollOver = [this, i, world] { rolloverMapButton(i, world); };
                btn->onRollOut = [this] { rolloutMapButton(); };
            }
            if (engine::MovieClip *ring = world->findChild("newring")) {
                ring->setVisible(user.getLevelHighScore(i) == 0);
                ring->mouseEnabled = false;
                ring->mouseChildren = false;
            }
            if (engine::MovieClip *ribbon = world->findChild("ribbon")) {
                ribbon->mouseEnabled = false;
                ribbon->mouseChildren = false;
                const data::CheckpointData *checkpoint = gameObj->obf_AQ->checkpointData.get();
                ribbon->setVisible(checkpoint != nullptr && checkpoint->whichLevel == i);
            }
        } else {
            world->setVisible(false);
        }
    }
    std::string label = as3::String(last + 1);
    if (last == 8 && user.getLevelHighScore(8) > 0) {
        label = user.getWarpCoins() == 50 ? "10" : "beat";
    }
    if (user.lastAreaRevealed == last) {
        map->gotoLabel("ready" + label, false);
        willRevealMap = false;
    } else {
        map->gotoLabel("unlock" + label, true);
        willRevealMap = true;
    }
    mapLastUnlocked = last;
}

// AS: MainMenuScreen.rolloverMapButton — the area's six challenges, done or not.
void MainMenuScreen::rolloverMapButton(int world, engine::MovieClip *worldClip)
{
    engine::MovieClip *map = section("map");
    engine::MovieClip *bubble = map ? map->findChild("rollover_bubble") : nullptr;
    engine::MovieClip *inside = bubble ? bubble->findChild("inside") : nullptr;
    if (!bubble || !inside) {
        return;
    }
    for (int i = 1; i <= 6; ++i) {
        engine::MovieClip *chal = inside->findChild("chal" + std::to_string(i));
        if (!chal) {
            continue;
        }
        chal->gotoAndStop(gameObj->obf_51->hasCompletedChallenge(world, i) ? 2 : 1);
        chal->setVisible(true);
        chal->setText("num_txt", std::to_string(i));
        engine::Affine at;
        if (world == 8) {
            if (i == 1) {
                inside->childMatrix("chal1", at);
                chal->setPosition(13, at.ty);
            } else {
                chal->setVisible(false);
            }
        } else if (world == 9) {
            chal->setVisible(false);
        } else if (i == 1) {
            inside->childMatrix("chal1", at);
            chal->setPosition(-22.75f, at.ty);
        }
    }
    bubble->setVisible(true);
    bubble->gotoAndPlay(1);
    engine::Affine at;
    if (map->childMatrix("world" + std::to_string(world + 1), at)) {
        bubble->setPosition(at.tx, at.ty);
    }
    (void)worldClip;
}

// AS: MainMenuScreen.rolloutMapButton
void MainMenuScreen::rolloutMapButton()
{
    if (engine::MovieClip *map = section("map")) {
        if (engine::MovieClip *bubble = map->findChild("rollover_bubble")) {
            bubble->setVisible(false);
        }
    }
}

// AS: MainMenuScreen.clickPlayMap — the area's customer choice (the API's MapSelectMenu screen).
void MainMenuScreen::clickPlayMap(int world)
{
    whichLevel = world;
    data::UserData &user = *gameObj->obf_51;
    user.hasRevealedLatestArea = true;
    if (mapLastUnlocked > user.lastAreaRevealed) {
        user.lastAreaRevealed = mapLastUnlocked;
    }
    gameObj->obf_7C->playSound("buttonclick.wav");
    gameObj->obf_c("nowarpkeys", false);
    isOpeningMapDetail = true;
    MainMenuParams next;
    next.section = "character";
    next.useLevel = whichLevel;
    next.isCharSelect = true;
    gameObj->showApiScreen("MapSelectMenu", next);
}

// AS: MainMenuScreen.clickStartLevel — the iris closes, then the level starts (closeMainMenuScreen).
void MainMenuScreen::clickStartLevel()
{
    isStartingLevel = true;
    gameObj->obf_c("challenges", false);
    gameObj->obf_64(false);
    data::UserData &user = *gameObj->obf_51;
    if (selectedCharacterIndex >= 0 && selectedCharacterIndex < static_cast<double>(user.customersUsed.size())) {
        user.customersUsed[static_cast<size_t>(selectedCharacterIndex)] = 1;
    }
    isClosing = true;
    if (engine::MovieClip *character = section("character")) {
        if (engine::MovieClip *iris = character->findChild("iris")) {
            iris->gotoLabel("irisout", true);
        }
    }
}

// AS: MainMenuScreen.setupCredits — the links to web sites are hidden (as the original does off its sites).
void MainMenuScreen::setupCredits()
{
    engine::MovieClip *credits = section("credits");
    if (!credits) {
        return;
    }
    for (const char *link : {"flipline1_btn", "flipline2_btn", "links", "facebook_btn", "twitter_btn"}) {
        if (engine::MovieClip *c = credits->findChild(link)) {
            c->setVisible(false);
        }
    }
}

// AS: MainMenuScreen.destroy
void MainMenuScreen::destroy()
{
    gameObj->obf_c("nowarpkeys", false);
    for (const char *event : {"clickCredits", "clickExit", "clickCreditsRedirect", "clickCreditsRedirectBack",
                              "clickMap", "clickStartLevel", "clickBackToMap", "clickExitMapSelect", "clickInfo",
                              "clickBackToGame", "clickQuit"}) {
        gameObj->removeContainerListener(event);
    }
    if (!isOnCharSelect) {
        setupInfo(false);
    }
    setupConfirmQuit(false);
    closeExitChoice();
    container->removeChild(clip);
    clip = nullptr;
}

// AS: MainMenuScreen.clickExit — back to the title screen. Port: first asks whether to go there or to the port's
// game picker (showExitChoice); the title screen choice runs the original code below.
void MainMenuScreen::clickExit()
{
    if (!exitChoice) {
        showExitChoice();
        return;
    }
    closeExitChoice();
    gameObj->obf_64(false);
    gameObj->showApiScreen("SplashScreen");
}

// Port (not in the original): a panel over the menu with TITLE SCREEN, GAME PICKER and CANCEL, built from the
// API's bottom panel and buttons. The menu and the bars take no clicks while it is open.
void MainMenuScreen::showExitChoice()
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    auto panel = engine::MovieClip::createEmpty(symbols);
    constexpr float kX = 145, kY = 118, kW = 410, kH = 180;
    // The bottom panel's striped band (700 x 101 at its origin) stretched over the panel, clipped by the whole
    // panel stretched to the panel's rectangle (its shapes fill all of their bounds).
    auto bg = engine::MovieClip::create(symbols, "api_menu_bottom_panel");
    auto mask = engine::MovieClip::create(symbols, "api_menu_bottom_panel");
    if (bg && mask) {
        float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
        mask->accumulateBounds(engine::Affine{}, minX, minY, maxX, maxY);
        if (maxX > minX && maxY > minY) {
            const float sx = kW / (maxX - minX), sy = kH / (maxY - minY);
            mask->setScale(sx, sy);
            mask->setPosition(kX - minX * sx, kY - minY * sy);
        }
        bg->setScale(kW / 700, kH / 101);
        bg->setPosition(kX, kY);
        bg->onClick = [] {}; // takes the clicks on the panel
        engine::MovieClip &maskRef = panel->addChild(std::move(mask), "");
        panel->addChild(std::move(bg), "").setMask(&maskRef);
    }
    engine::MovieClip::TextField title;
    title.font = "Arial Black";
    title.size = 22;
    title.color = SDL_FColor{0.69f, 0.69f, 0.68f, 1};
    title.x = kX;
    title.y = kY + 18;
    title.width = kW;
    title.align = 2;
    title.text = gameObj->lang->get("exit.title");
    panel->addTextField("title", title);
    const auto button = [&](const char *key, float x, float y, float w, std::function<void()> fn) {
        if (auto b = engine::makeApiButton(symbols, gameObj->text, gameObj->lang->get(key), false, w)) {
            b->setPosition(x, y);
            b->onClick = std::move(fn);
            panel->addChild(std::move(b), "");
        }
    };
    button("exit.title_screen", kX + 15, kY + 72, 180, [this] { clickExit(); });
    button("exit.frontend", kX + 215, kY + 72, 180, [this] {
        closeExitChoice();
        gameObj->wantsFrontend = true; // the app goes back to its game picker after this frame
    });
    button("exit.cancel", kX + 135, kY + 124, 140, [this] { closeExitChoice(); });
    exitChoice = &container->addChild(std::move(panel), "");
    clip->mouseChildren = false;
    gameObj->menuBar.setEnabled(false);
}

void MainMenuScreen::closeExitChoice()
{
    if (!exitChoice) {
        return;
    }
    container->removeChild(exitChoice);
    exitChoice = nullptr;
    if (clip) {
        clip->mouseChildren = true;
    }
    gameObj->menuBar.setEnabled(true);
}

// AS: MainMenuScreen.clickExitMapSelect
void MainMenuScreen::clickExitMapSelect()
{
    gameObj->obf_64(false);
    gameObj->showApiScreen("SplashScreen");
}

// AS: MainMenuScreen.clickBackToMap
void MainMenuScreen::clickBackToMap()
{
    isReturningToMap = true;
    gameObj->obf_64(false);
    MainMenuParams next;
    next.section = "map";
    gameObj->showApiScreen("MainMenu", next);
}

// AS: MainMenuScreen.setSection — the old section slides up and away, the new one slides in from below.
void MainMenuScreen::setSection(const std::string &name)
{
    if (name == currentSection) {
        return;
    }
    newSection = name;
    for (const char *s : {"map", "info", "character", "baddies", "controls", "medals", "credits", "help"}) {
        if (engine::MovieClip *c = section(s)) {
            c->setVisible(false);
        }
    }
    if (!currentSection.empty()) {
        if (engine::MovieClip *c = section(currentSection)) {
            c->setVisible(true);
            c->setY(0);
        }
    }
    if (newSection != "none") {
        if (engine::MovieClip *c = section(newSection)) {
            c->setVisible(true);
            c->setY(480);
        }
    }
    if (currentSection == "controls") {
        cancelSettingControls();
    }
    if (newSection == "help") {
        setupHelpKeys();
    }
    if (newSection == "map" && willRevealMap) {
        gameObj->obf_7C->playSound("checkpoint.wav");
    }
    isTransitioning = true;
    tweening_ = true;
}

// AS: MainMenuScreen.tweenSections (ENTER_FRAME)
void MainMenuScreen::tweenSections()
{
    double oldDistance = 999;
    engine::MovieClip *old = currentSection.empty() ? nullptr : section(currentSection);
    if (old) {
        oldDistance = -480 - old->y();
        old->setY(static_cast<float>(old->y() + oldDistance / tweenSpeed));
    }
    if (newSection != "none") {
        engine::MovieClip *incoming = section(newSection);
        const double distance = incoming ? 0 - incoming->y() : 0;
        if (incoming) {
            incoming->setY(static_cast<float>(incoming->y() + distance / tweenSpeed));
        }
        if (std::abs(distance) <= 1) {
            if (incoming) {
                incoming->setY(0);
            }
            showSectionTraining();
            if (old) {
                old->setVisible(false);
            }
            currentSection = newSection;
            newSection.clear();
            isTransitioning = false;
            tweening_ = false;
            gameObj->menuBar.setTitle(gameObj->lang, getSectionTitle());
            gameObj->menuBar.setBottomHidden(currentSection == "credits"); // port: the credits fill the stage
        }
    } else if (std::abs(oldDistance) <= 1) {
        isTransitioning = false;
        tweening_ = false;
        if (isClosing) {
            closeMainMenuScreen();
        }
    }
}

// Port: the training bubbles tweenSections opens once a section has slid in (its debug trace() calls are left out).
void MainMenuScreen::showSectionTraining()
{
    data::UserData &user = *gameObj->obf_51;
    if (newSection == "character") {
        if (!user.hasTrained("character") && user.getTotalCustomersUnlocked() > 2) {
            gameObj->obf_6n("character", false);
        } else if (!user.hasTrained("challenges") && user.hasTrained("nowarpkeys")) {
            gameObj->obf_6n("challenges", false);
        } else if (!user.hasTrained("styles") && user.getLevelHighScore(2) > 0 &&
                   user.getTotalMoney() >= gameObj->obf_AQ->getOutfitPrice(user.selectedCharacter, 1)) {
            gameObj->obf_6n("styles", false);
        }
    } else if (newSection == "map") {
        if (!user.hasTrained("nowarpkeys") && !willRevealMap && user.getLevelHighScore(user.lastAreaRevealed) > 0) {
            gameObj->obf_6n("nowarpkeys", false);
        }
    }
}

// AS: MainMenuScreen.startClosingScreen
void MainMenuScreen::startClosingScreen()
{
    isClosing = true;
    gameObj->menuBar.clear(); // api.§_-69§: the open API bars transition out
    setSection("none");
}

// AS: MainMenuScreen.closeMainMenuScreen — start the level, quit it, go back, or resume the game. The analytics
// and leaderboard calls are dropped.
void MainMenuScreen::closeMainMenuScreen()
{
    if (isStartingLevel) {
        SDL_Log(">>>>>>>>>>>>>>>>>>>>>>>>>> SETUP GAME");
        gameObj->obf_8W = whichLevel;
        gameObj->obf_9k();
        gameObj->showApiScreen(""); // removes this screen
    } else if (isOpeningMapDetail) {
        MainMenuParams next;
        next.section = "area";
        next.useLevel = whichLevel;
        gameObj->showApiScreen("MapSelectMenu", next);
    } else if (isQuittingLevel) {
        if (!gameObj->obf_7C->isMute) {
            gameObj->obf_7C->unmuteSound(false);
        }
        if (gameObj->obf_8o) {
            gameObj->obf_51->totalTimePlayed += gameObj->obf_8o->currentLevelTimer;
            SDL_Log("(Updated Time Played)");
        }
        gameObj->obf_51->saveProgress("quitlevel");
        gameObj->obf_Ap();
        MainMenuParams next;
        next.section = "map";
        gameObj->showApiScreen("MainMenu", next); // also closes the PauseMenu (this screen)
    } else if (isReturningToMap || isContinuingToMap) {
        MainMenuParams next;
        next.section = "map";
        gameObj->showApiScreen("MainMenu", next);
    } else {
        gameObj->obf_8o->resumeGame(keysWereChanged);
        gameObj->showApiScreen(""); // api.§_-61§("PauseMenu")
    }
}

// AS: MainMenuScreen.clickBackToGame
void MainMenuScreen::clickBackToGame()
{
    if (engine::MovieClip *bg = clip->findChild("blackbg")) {
        bg->setVisible(false);
    }
    startClosingScreen();
}

// AS: MainMenuScreen.clickQuit — "are you sure?" over the menu; the bars stop taking clicks.
void MainMenuScreen::clickQuit()
{
    if (engine::MovieClip *confirm = section("confirmquit")) {
        confirm->setVisible(true);
    }
    gameObj->menuBar.setEnabled(false);
}

// AS: MainMenuScreen.clickConfirmQuit
void MainMenuScreen::clickConfirmQuit()
{
    isQuittingLevel = true;
    if (engine::MovieClip *confirm = section("confirmquit")) {
        confirm->setVisible(false);
    }
    gameObj->menuBar.setEnabled(true); // port: the bar is cleared next (the API's buttons are rebuilt per screen)
    startClosingScreen();
}

// AS: MainMenuScreen.clickCancelQuit
void MainMenuScreen::clickCancelQuit()
{
    if (engine::MovieClip *confirm = section("confirmquit")) {
        confirm->setVisible(false);
    }
    gameObj->menuBar.setEnabled(true);
}

// AS: MainMenuScreen.setupConfirmQuit — the YES / NO buttons (the API's small buttons, 80 wide).
void MainMenuScreen::setupConfirmQuit(bool setup)
{
    engine::MovieClip *confirm = section("confirmquit");
    if (!confirm) {
        return;
    }
    if (setup) {
        std::unique_ptr<engine::MovieClip> yes = engine::makeApiButton(*gameObj->symbols, gameObj->text,
                                                                       gameObj->lang->get("menu.yes"), false, 80);
        std::unique_ptr<engine::MovieClip> no = engine::makeApiButton(*gameObj->symbols, gameObj->text,
                                                                      gameObj->lang->get("menu.no"), false, 80);
        if (yes && no) {
            yes->setPosition(234, 220);
            no->setPosition(385, 220);
            yes->onClick = [this] { clickConfirmQuit(); };
            no->onClick = [this] { clickCancelQuit(); };
            confirmYesButton = &confirm->addChild(std::move(yes), "");
            confirmNoButton = &confirm->addChild(std::move(no), "");
        }
        confirm->setVisible(false);
    } else {
        if (confirmYesButton) {
            confirm->removeChild(confirmYesButton);
            confirmYesButton = nullptr;
        }
        if (confirmNoButton) {
            confirm->removeChild(confirmNoButton);
            confirmNoButton = nullptr;
        }
    }
}

// AS: MainMenuScreen.setupInfo — the pause menu's AREA INFO: the area, the customer being played, the six
// challenges (with their tallies), and this level's and the game's totals.
void MainMenuScreen::setupInfo(bool setup)
{
    engine::MovieClip *info = section("info");
    if (!info) {
        return;
    }
    data::UserData &user = *gameObj->obf_51;
    data::DataManager &dm = *gameObj->obf_AQ;
    if (setup && dm.currentWorldData != nullptr) {
        const double level = dm.currentLevel;
        info->setText("title_txt", dm.getWorldTitle(level));
        info->setText("levelnum_txt", level == 9 ? "X" : as3::String(level + 1));
        info->setText("score_txt", obf_AF::obf_81::obf_7w(user.getLevelHighScore(level)));
        const data::CustomerDataFile *c = gameObj->obf_3A->getCustomerData(user.selectedCharacter);
        if (engine::MovieClip *style = info->findChild("currentstyle")) {
            style->gotoAndStop(static_cast<int>(user.selectedStyle));
        }
        if (c) {
            info->setText("name_txt", c->customerName);
            info->setText("weaponname_txt",
                          c->weaponName == "Pizza Paddle" && user.selectedStyle == 3 ? "Beach Umbrella" : c->weaponName);
            if (engine::MovieClip *icon = info->findChild("skillicon")) {
                icon->gotoLabel(c->skillType, false);
            }
            const std::string skill = c->skillType;
            const char *key = skill == CustomerData::SKILL_CRAWL        ? "skill.crawl"
                              : skill == CustomerData::SKILL_DOUBLEJUMP ? "skill.doublejump"
                              : skill == CustomerData::SKILL_WALLJUMP   ? "skill.walljump"
                              : skill == CustomerData::SKILL_GLIDE      ? "skill.glide"
                              : skill == CustomerData::SKILL_POUND      ? "skill.pound"
                              : skill == CustomerData::SKILL_PUSH       ? "skill.push"
                                                                        : "skill.none";
            info->setText("skillname_txt", gameObj->lang->get(key));
        }
        SDL_Log("Setup Challenges for level: %s", as3::String(level).c_str());
        for (int i = 1; i <= 6; ++i) {
            engine::MovieClip *icon = info->findChild("icon" + std::to_string(i));
            engine::MovieClip *panel = info->findChild("panel" + std::to_string(i));
            if (!icon || !panel) {
                continue;
            }
            icon->gotoAndStop(user.hasCompletedChallenge(level, i) ? 2 : 1);
            icon->setText("num_txt", std::to_string(i));
            const std::string type = gameObj->obf_94->getChallengeType(level, i);
            const std::string skill = gameObj->obf_94->getChallengeSkillNeeded(level, i);
            engine::MovieClip *panelIcon = panel->findChild("icon");
            if (type == data::Challenge::RESCUE) {
                panel->setText("description_txt", gameObj->lang->get("challenge.rescue"));
                const double trapped = gameObj->obf_3A->getTrappedCustomerIndex(level, i);
                engine::MovieClip::BitmapChild face = gameObj->obf_3A->getCustomerBitmap(trapped);
                face.x = -8;
                face.y = -8;
                const std::string name = gameObj->obf_3A->getCustomerName(trapped);
                if (name == "Georgito" || name == "Yippy" || name == "Greg") {
                    face.y -= 8;
                }
                if (panelIcon) {
                    panelIcon->gotoAndStop(1);
                    if (engine::MovieClip *holder = panelIcon->findChild("holder")) {
                        holder->addBitmap(face);
                        holder->setMask(panelIcon->findChild("masker"));
                    }
                }
            } else {
                panel->setText("description_txt", gameObj->obf_94->getChallengeTallyString(level, i));
                if (panelIcon) {
                    panelIcon->gotoAndStop(type == data::Challenge::BURGERZILLAS ? 2
                                           : type == data::Challenge::COINS      ? 3
                                                                                 : 4 + static_cast<int>(level));
                }
            }
            engine::MovieClip *skillClip = panel->findChild("skill");
            if (skill == CustomerData::SKILL_NONE || skill.empty()) {
                if (skillClip) {
                    skillClip->stop();
                    skillClip->setVisible(false);
                }
                if (engine::MovieClip *needs = panel->findChild("needs")) {
                    needs->setVisible(false);
                }
            } else if (skillClip) {
                skillClip->gotoLabel(skill, false);
            }
            if (type.empty()) {
                panel->setVisible(false);
                icon->setVisible(false);
            }
        }
        info->setText("points_txt", obf_AF::obf_81::obf_7w(user.getCurrentPoints()));
        const double timer = gameObj->obf_8o ? gameObj->obf_8o->gameplayTimer : 0;
        info->setText("time_txt", obf_AF::obf_81::obf_2i(timer, true));
        info->setText("coins_txt", obf_AF::obf_81::obf_7w(user.getCurrentMoney()));
        info->setText("totalpoints_txt", obf_AF::obf_81::obf_7w(user.getTotalScore()));
        info->setText("totaltime_txt", obf_AF::obf_81::obf_2i(user.totalTimePlayed, true, true));
        info->setText("totalcoins_txt", obf_AF::obf_81::obf_7w(user.getTotalMoney()));
        info->setText("totalwarpcoins_txt", as3::String(user.getWarpCoins()) + "/50");
        info->setText("totalcustomers_txt", as3::String(user.getTotalCustomersUnlocked()) + "/28");
        if (std::unique_ptr<engine::MovieClip> model = buildInfoModel(user.selectedCharacter, user.selectedStyle)) {
            model->setPosition(555, 80);
            model->mouseEnabled = false;
            model->mouseChildren = false;
            infoModel = &info->addChild(std::move(model), "");
        }
    } else if (setup) {
        SDL_Log("No area screen.");
    } else if (infoModel) {
        info->removeChild(infoModel);
        infoModel = nullptr;
    }
}

// AS: MainMenuScreen.buildInfoModel — the customer being played, standing, in the info section.
std::unique_ptr<engine::MovieClip> MainMenuScreen::buildInfoModel(double whichCharacter, double style)
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    const std::string type = gameObj->obf_3A->getCustomerType(whichCharacter);
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
    std::unique_ptr<engine::MovieClip> model = engine::MovieClip::create(symbols, puppet);
    if (!model) {
        return nullptr;
    }
    model->setScale(-0.55f, 0.55f);
    const std::string clipName = gameObj->obf_3A->getCustomerClipName(whichCharacter);
    std::string name = clipName;
    if (style == 2) {
        name += "2";
    } else if (style == 3) {
        name += "3";
    }
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
    const std::string weapon = "weapon_" + gameObj->obf_3A->getWeaponClipName(whichCharacter, style);
    attach("weapon", weapon);
    attach("cross_back_weap", weapon);
    if (clipName == "Boomer") {
        attach("glider", "glider_" + clipName);
    } else if (engine::MovieClip *glider = model->findChild("glider")) {
        glider->addChild(engine::MovieClip::createEmpty(symbols), "clip");
    }
    model->gotoAndStop(1);
    model->gotoLabel("stand", true);
    if (clipName == "Connor") {
        model->gotoLabel("standconnor", true);
    }
    return model;
}

// AS: MainMenuScreen.getSectionTitle (a translation key in the port)
std::string MainMenuScreen::getSectionTitle() const
{
    if (currentSection == "map") return "title.map";
    if (currentSection == "controls") return "menu.controls";
    if (currentSection == "baddies") return "menu.baddies";
    if (currentSection == "credits") return "menu.credits";
    if (currentSection == "medals") return "menu.badges";
    if (currentSection == "help") return "menu.help";
    if (currentSection == "info") return "title.area_info";
    if (currentSection == "character") return "title.choose_your_character";
    return currentSection;
}

void MainMenuScreen::enterFrame()
{
    if (tweening_) {
        tweenSections();
    }
    if (updatingChar_ && clip) {
        updateCharacter();
    }
    if (animatingMedals_ && clip) {
        animateMedalsTransition();
    }
    if (clip) {
        updateHelp();
    }
}

// AS: MainMenuScreen.setupBaddies — the 35 enemy thumbnails: the ones met so far are shown (met / total), the
// first met one of Onion, Brown Onion or Kaiser Onion is selected. The original draws each thumbnail
// (enemythumb_<clip>, with a drop shadow) into one bitmap (DataManager.generateEnemyBitmap / getEnemyBitmap); the port
// adds the clip itself, without the shadow. (Removing the listeners, setup false, is left to the clip going away.)
void MainMenuScreen::setupBaddies(bool setup)
{
    const data::UserData &user = *gameObj->obf_51;
    const data::DataManager &dm = *gameObj->obf_AQ;
    engine::MovieClip *baddies = section("baddies");
    if (!setup || !baddies || dm.enemyKey.empty()) {
        return;
    }
    double met = 0;
    double total = 0;
    for (int i = 0; i < 35; ++i) {
        engine::MovieClip *thumb = baddies->findChild("thumb" + std::to_string(i));
        if (!thumb) {
            continue;
        }
        if (i < static_cast<int>(dm.enemyKey.size())) {
            ++total;
            const data::DataManager::EnemyKeyEntry &key = dm.enemyKey[static_cast<size_t>(i)];
            for (const char *part : {"hilite", "roll", "newbanner"}) {
                if (engine::MovieClip *c = thumb->findChild(part)) {
                    c->setVisible(false);
                }
            }
            thumb->onMouseDown = [this, i] { clickEnemyThumb(i); };
            // AS: MainMenuScreen.rolloverEnemyThumb
            thumb->onRollOver = [thumb] {
                if (engine::MovieClip *roll = thumb->findChild("roll")) {
                    roll->setVisible(true);
                }
            };
            // AS: MainMenuScreen.rolloutEnemyThumb
            thumb->onRollOut = [thumb] {
                if (engine::MovieClip *roll = thumb->findChild("roll")) {
                    roll->setVisible(false);
                }
            };
            thumb->gotoAndStop(6);
            if (key.id == 35 || key.id == 37) {
                thumb->gotoAndStop(2);
            }
            if (engine::MovieClip *holder = thumb->findChild("holder")) {
                if (auto pic = engine::MovieClip::create(*gameObj->symbols, "enemythumb_" + key.clip)) {
                    pic->mouseEnabled = false;
                    holder->addChild(std::move(pic), "");
                }
            }
            if (user.getEnemyKills(key.id) <= 0) {
                thumb->setVisible(false);
            } else {
                ++met;
            }
        } else {
            thumb->setVisible(false);
        }
    }
    baddies->setText("tally_txt", as3::String(met) + "/" + as3::String(total));
    if (user.getEnemyKills(dm.enemyKey[0].id) > 0) {
        selectEnemy(0);
    } else if (dm.enemyKey.size() > 1 && user.getEnemyKills(dm.enemyKey[1].id) > 0) {
        selectEnemy(1);
    } else if (dm.enemyKey.size() > 4 && user.getEnemyKills(dm.enemyKey[4].id) > 0) {
        selectEnemy(4);
    } else {
        selectEnemy(0);
    }
}

// AS: MainMenuScreen.clickEnemyThumb (MOUSE_DOWN; the thumbnail's number comes from its instance name)
void MainMenuScreen::clickEnemyThumb(int index)
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    selectEnemy(index);
}

// AS: MainMenuScreen.selectEnemy — its thumbnail lit, its name and count (encounters for Swiss Zack, bounces for
// Jellyback), and its picture.
void MainMenuScreen::selectEnemy(double index)
{
    const data::DataManager &dm = *gameObj->obf_AQ;
    const engine::Lang &lang = *gameObj->lang;
    engine::MovieClip *baddies = section("baddies");
    if (!baddies) {
        return;
    }
    for (int i = 0; i < 35; ++i) {
        if (engine::MovieClip *thumb = baddies->findChild("thumb" + std::to_string(i))) {
            if (engine::MovieClip *hilite = thumb->findChild("hilite")) {
                hilite->setVisible(i == index);
            }
        }
    }
    baddies->setText("name_txt", dm.getEnemyName(index));
    const double id = dm.getEnemyIDFromIndex(index);
    const std::string kills = as3::String(gameObj->obf_51->getEnemyKills(id));
    if (id == 25) {
        baddies->setText("defeated_txt", lang.get("baddies.encounters", kills));
    } else if (id == 32) {
        baddies->setText("defeated_txt", lang.get("baddies.bounced", kills));
    } else {
        baddies->setText("defeated_txt", lang.get("baddies.defeated", kills));
    }
    if (enemyDetail != nullptr) {
        baddies->removeChild(enemyDetail);
        enemyDetail = nullptr;
    }
    if (auto detail = engine::MovieClip::create(*gameObj->symbols, "enemydetail_" + dm.getEnemyClipName(index))) {
        detail->mouseEnabled = false;
        detail->mouseChildren = false;
        detail->setPosition(10, 101);
        enemyDetail = &baddies->addChild(std::move(detail), "");
    }
}

namespace {
const char *const kControlKeys[][2] = {{"attack", papaGame::data::DataManager::KEY_ATTACK},
                                       {"jump", papaGame::data::DataManager::KEY_JUMP},
                                       {"down", papaGame::data::DataManager::KEY_DOWN},
                                       {"up", papaGame::data::DataManager::KEY_UP},
                                       {"left", papaGame::data::DataManager::KEY_LEFT},
                                       {"right", papaGame::data::DataManager::KEY_RIGHT}};
} // namespace

// AS: MainMenuScreen.setupControls — each key's label and its button (click to change it; roll-over lights it).
// (Removing the listeners, setup false, is left to the clip going away.)
void MainMenuScreen::setupControls(bool setup)
{
    const data::DataManager &dm = *gameObj->obf_AQ;
    engine::MovieClip *controls = section("controls");
    if (!setup || !controls) {
        return;
    }
    for (const auto &k : kControlKeys) {
        const std::string which = k[0];
        controls->setText(which + "_txt", dm.getKeyLabel(k[1]));
        engine::MovieClip *btn = controls->findChild(which + "_btn");
        if (!btn) {
            continue;
        }
        btn->onClick = [this, which] { clickControlsButton(which); };
        // AS: MainMenuScreen.rolloverControlsButton / rolloutControlsButton
        btn->onRollOver = [this, btn] {
            if (settingWhichKey == "none") {
                btn->gotoLabel("rollover", false);
            }
        };
        // AS: MainMenuScreen.rolloutControlsButton
        btn->onRollOut = [this, btn] {
            if (settingWhichKey == "none") {
                btn->gotoLabel("click", false);
            }
        };
        btn->gotoLabel("click", false);
    }
}

// AS: MainMenuScreen.clickControlsButton — the button asks for a key (the button's name gives the action).
void MainMenuScreen::clickControlsButton(const std::string &which)
{
    if (settingWhichKey == "none") {
        if (engine::MovieClip *btn = section("controls")->findChild(which + "_btn")) {
            btn->gotoLabel("presskey", false);
        }
        settingWhichKey = which;
    }
}

// AS: MainMenuScreen.cancelSettingControls
void MainMenuScreen::cancelSettingControls()
{
    if (settingWhichKey != "none") {
        if (engine::MovieClip *controls = section("controls")) {
            for (const auto &k : kControlKeys) {
                if (engine::MovieClip *btn = controls->findChild(std::string(k[0]) + "_btn")) {
                    btn->gotoLabel("click", false);
                }
            }
        }
        settingWhichKey = "none";
    }
}

// AS: MainMenuScreen.controlsKeyListener (KEY_DOWN) — the key is taken unless another action uses it; the labels
// are refreshed. The "ChangedControls" analytics event is dropped.
void MainMenuScreen::controlsKeyListener(double keyCode)
{
    const data::DataManager &dm = *gameObj->obf_AQ;
    engine::MovieClip *controls = section("controls");
    const bool ok = gameObj->obf_51->setKey(keyCode, settingWhichKey);
    engine::MovieClip *btn = controls ? controls->findChild(settingWhichKey + "_btn") : nullptr;
    if (ok) {
        if (btn) {
            btn->gotoLabel("click", false);
        }
        for (const auto &k : kControlKeys) {
            controls->setText(std::string(k[0]) + "_txt", dm.getKeyLabel(k[1]));
        }
        keysWereChanged = true;
    } else if (btn) {
        btn->gotoLabel("alreadyused", false);
    }
    settingWhichKey = "none";
}

namespace {
// Port: DisplayObject.height of a clip (its shapes and children, in its parent's units).
double clipHeight(const engine::MovieClip &c)
{
    float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
    c.accumulateBounds(engine::Affine{}, minX, minY, maxX, maxY);
    return maxY > minY ? maxY - minY : 0.0;
}

// Port: `<child>.y = y` on a timeline child: its x stays where the timeline put it.
void setChildY(engine::MovieClip &parent, engine::MovieClip *child, const char *name, double y)
{
    if (!child) {
        return;
    }
    if (!child->hasPosition()) {
        engine::Affine m;
        child->setPosition(parent.childMatrix(name, m) ? m.tx : 0.0f, static_cast<float>(y));
    } else {
        child->setY(static_cast<float>(y));
    }
}

// Port: `<child>.y` of a timeline child (its position on the current frame, or the one code set).
double childY(engine::MovieClip &parent, engine::MovieClip *child, const char *name)
{
    if (child && child->hasPosition()) {
        return child->y();
    }
    engine::Affine m;
    return parent.childMatrix(name, m) ? m.ty : 0.0;
}
} // namespace

// AS: MainMenuScreen.setupHelp — the 18 topic tabs (names in the port's text tables), the key pictures, the two
// masked, scrolled panels and their scroll bars; topic 1 shown. (The original's `useSection: "help"` parameter, which
// opens topic 15, is never sent in the port. Removing the listeners, setup false, is left to the clip going away.)
void MainMenuScreen::setupHelp(bool setup)
{
    engine::MovieClip *help = section("help");
    engine::MovieClip *tabholder = help ? help->findChild("tabholder") : nullptr;
    if (!setup || !tabholder) {
        return;
    }
    for (int i = 1; i <= 18; ++i) {
        engine::MovieClip *tab = tabholder->findChild("tab" + std::to_string(i));
        if (!tab) {
            continue;
        }
        tab->onMouseDown = [this, i] { clickHelpTab(i); };
        // AS: MainMenuScreen.rolloverHelpTab / rolloutHelpTab
        tab->onRollOver = [tab] {
            if (engine::MovieClip *h = tab->findChild("hilite")) {
                h->setVisible(true);
            }
        };
        // AS: MainMenuScreen.rolloutHelpTab
        tab->onRollOut = [this, tab, i] {
            if (helpIndex != i) {
                if (engine::MovieClip *h = tab->findChild("hilite")) {
                    h->setVisible(false);
                }
            }
        };
        tab->mouseChildren = false;
        tab->setText("name_txt", "<b>" + gameObj->lang->get("help.topic" + std::to_string(i)) + "</b>");
        for (const char *part : {"hilite", "arrow"}) {
            if (engine::MovieClip *c = tab->findChild(part)) {
                c->setVisible(false);
            }
        }
    }
    setupHelpKeys();
    tabholder->setMask(help->findChild("tabmasker"));
    if (engine::MovieClip *panel = help->findChild("side_scrollpanel")) {
        if (engine::MovieClip *btn = panel->findChild("scroll_btn")) {
            btn->onMouseDown = [this] { clickHelpTabDragger(); };
            setChildY(*panel, btn, "scroll_btn", helpScrollStart);
        }
        if (engine::MovieClip *up = panel->findChild("up_btn")) {
            up->onMouseDown = [this] { clickHelpTabArrow(-1); };
        }
        if (engine::MovieClip *down = panel->findChild("down_btn")) {
            down->onMouseDown = [this] { clickHelpTabArrow(1); };
        }
    }
    if (engine::MovieClip *mainpanel = help->findChild("mainpanel")) {
        mainpanel->setMask(help->findChild("mainmasker"));
    }
    if (engine::MovieClip *panel = help->findChild("main_scrollpanel")) {
        if (engine::MovieClip *btn = panel->findChild("scroll_btn")) {
            btn->onMouseDown = [this] { clickHelpMainDragger(); };
            setChildY(*panel, btn, "scroll_btn", helpScrollStart);
        }
        if (engine::MovieClip *up = panel->findChild("up_btn")) {
            up->onMouseDown = [this] { clickHelpMainArrow(-1); };
        }
        if (engine::MovieClip *down = panel->findChild("down_btn")) {
            down->onMouseDown = [this] { clickHelpMainArrow(1); };
        }
    }
    showHelp(1);
}

// AS: MainMenuScreen.setupHelpKeys — the help pictures show the player's keys: an arrow, a wide key with the name,
// or a letter key; none when the key has no label.
void MainMenuScreen::setupHelpKeys()
{
    using data::DataManager;
    static const char *const names[] = {"left", "right", "up", "down", "down2", "down3", "jump", "jump2", "attack"};
    static const char *const keys[] = {DataManager::KEY_LEFT, DataManager::KEY_RIGHT, DataManager::KEY_UP,
                                       DataManager::KEY_DOWN, DataManager::KEY_DOWN,  DataManager::KEY_DOWN,
                                       DataManager::KEY_JUMP, DataManager::KEY_JUMP,  DataManager::KEY_ATTACK};
    engine::MovieClip *help = section("help");
    engine::MovieClip *mainpanel = help ? help->findChild("mainpanel") : nullptr;
    if (!mainpanel) {
        return;
    }
    for (size_t i = 0; i < std::size(names); ++i) {
        engine::MovieClip *key = mainpanel->findChild(std::string("key_") + names[i]);
        if (!key) {
            continue;
        }
        const std::string label = gameObj->obf_AQ->getKeyLabel(keys[i]);
        if (!label.empty()) {
            key->setVisible(true);
            if (label == "Left" || label == "Right" || label == "Up" || label == "Down") {
                std::string lower = label;
                for (char &c : lower) {
                    c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
                }
                key->gotoLabel(lower, false);
                key->setChildAlpha("letter_txt", 0);
                key->setChildAlpha("word_txt", 0);
            } else if (label.size() > 1) {
                key->gotoLabel("wide", false);
                key->setChildAlpha("letter_txt", 0);
                key->setChildAlpha("word_txt", 1);
                key->setText("word_txt", label);
            } else {
                key->gotoLabel("other", false);
                key->setChildAlpha("letter_txt", 1);
                key->setText("letter_txt", label);
                key->setChildAlpha("word_txt", 0);
            }
        } else {
            key->setVisible(false);
        }
    }
}

// AS: MainMenuScreen.showHelp — the topic's tab lit, its page at the top.
void MainMenuScreen::showHelp(double index)
{
    engine::MovieClip *help = section("help");
    if (!help) {
        return;
    }
    helpIndex = index;
    if (engine::MovieClip *tabholder = help->findChild("tabholder")) {
        for (int i = 1; i <= 18; ++i) {
            if (engine::MovieClip *tab = tabholder->findChild("tab" + std::to_string(i))) {
                for (const char *part : {"hilite", "arrow"}) {
                    if (engine::MovieClip *c = tab->findChild(part)) {
                        c->setVisible(i == index);
                    }
                }
            }
        }
    }
    if (engine::MovieClip *mainpanel = help->findChild("mainpanel")) {
        mainpanel->gotoAndStop(static_cast<int>(helpIndex));
        setChildY(*help, mainpanel, "mainpanel", childY(*help, help->findChild("mainmasker"), "mainmasker"));
    }
    if (engine::MovieClip *panel = help->findChild("main_scrollpanel")) {
        if (engine::MovieClip *btn = panel->findChild("scroll_btn")) {
            setChildY(*panel, btn, "scroll_btn", helpScrollStart);
        }
    }
}

// AS: MainMenuScreen.clickHelpTab (MOUSE_DOWN; the tab's number comes from its instance name)
void MainMenuScreen::clickHelpTab(int index)
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    showHelp(index);
}

// AS: MainMenuScreen.clickHelpTabArrow — the tab list scrolls while the arrow is held.
void MainMenuScreen::clickHelpTabArrow(int dir)
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    helpTabScrollDir = dir;
    holdingTabArrow_ = true;
}

// AS: MainMenuScreen.holdHelpTabArrow (ENTER_FRAME) — 8 px a frame within the mask; the scroll button follows.
void MainMenuScreen::holdHelpTabArrow()
{
    engine::MovieClip *help = section("help");
    engine::MovieClip *holder = help ? help->findChild("tabholder") : nullptr;
    engine::MovieClip *masker = help ? help->findChild("tabmasker") : nullptr;
    if (!holder || !masker) {
        return;
    }
    const double top = childY(*help, masker, "tabmasker");
    const double bottom = top - (clipHeight(*holder) - clipHeight(*masker));
    double y = childY(*help, holder, "tabholder");
    if (helpTabScrollDir == 1) {
        y -= 8;
    } else if (helpTabScrollDir == -1) {
        y += 8;
    }
    if (y < bottom) {
        y = bottom;
        helpTabScrollDir = 0;
    } else if (y > top) {
        y = top;
        helpTabScrollDir = 0;
    }
    setChildY(*help, holder, "tabholder", y);
    const double ratio = bottom != top ? std::abs((y - top) / (bottom - top)) : 0;
    if (engine::MovieClip *panel = help->findChild("side_scrollpanel")) {
        if (engine::MovieClip *btn = panel->findChild("scroll_btn")) {
            setChildY(*panel, btn, "scroll_btn", helpScrollStart + ratio * helpScrollRange);
        }
    }
}

// AS: MainMenuScreen.releaseHelpTabArrow (stage MOUSE_UP)
void MainMenuScreen::releaseHelpTabArrow()
{
    helpTabScrollDir = 0;
    holdingTabArrow_ = false;
}

// AS: MainMenuScreen.clickHelpTabDragger — the scroll button is dragged along its 158 px track.
void MainMenuScreen::clickHelpTabDragger()
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    engine::MovieClip *btn = section("help")->findChild("side_scrollpanel")->findChild("scroll_btn");
    dragStartPointerY_ = gameObj->pointer.y;
    dragStartButtonY_ = btn->y();
    draggingTab_ = true;
}

// AS: MainMenuScreen.dragHelpTabDragger (ENTER_FRAME) — the tab list follows the button.
void MainMenuScreen::dragHelpTabDragger()
{
    engine::MovieClip *help = section("help");
    engine::MovieClip *btn_parent = help->findChild("side_scrollpanel");
    engine::MovieClip *btn = btn_parent->findChild("scroll_btn");
    engine::MovieClip *holder = help->findChild("tabholder");
    engine::MovieClip *masker = help->findChild("tabmasker");
    // startDrag: the button keeps its offset to the pointer, inside Rectangle(0, 30, 0, helpScrollRange).
    const double by = std::clamp(dragStartButtonY_ + (gameObj->pointer.y - dragStartPointerY_), 30.0,
                                 30.0 + helpScrollRange);
    setChildY(*btn_parent, btn, "scroll_btn", by);
    const double ratio = (by - helpScrollStart) / helpScrollRange;
    if (holder && masker) {
        setChildY(*help, holder, "tabholder",
                  childY(*help, masker, "tabmasker") - (clipHeight(*holder) - clipHeight(*masker)) * ratio);
    }
}

// AS: MainMenuScreen.releaseHelpTabDragger (stage MOUSE_UP)
void MainMenuScreen::releaseHelpTabDragger()
{
    draggingTab_ = false;
}

// AS: MainMenuScreen.clickHelpMainArrow
void MainMenuScreen::clickHelpMainArrow(int dir)
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    helpMainScrollDir = dir;
    holdingMainArrow_ = true;
}

// AS: MainMenuScreen.holdHelpMainArrow (ENTER_FRAME)
void MainMenuScreen::holdHelpMainArrow()
{
    engine::MovieClip *help = section("help");
    engine::MovieClip *panel = help ? help->findChild("mainpanel") : nullptr;
    engine::MovieClip *masker = help ? help->findChild("mainmasker") : nullptr;
    if (!panel || !masker) {
        return;
    }
    const double top = childY(*help, masker, "mainmasker");
    const double bottom = top - (clipHeight(*panel) - clipHeight(*masker));
    double y = childY(*help, panel, "mainpanel");
    if (helpMainScrollDir == 1) {
        y -= 8;
    } else if (helpMainScrollDir == -1) {
        y += 8;
    }
    if (y < bottom) {
        y = bottom;
        helpMainScrollDir = 0;
    } else if (y > top) {
        y = top;
        helpMainScrollDir = 0;
    }
    setChildY(*help, panel, "mainpanel", y);
    const double ratio = bottom != top ? std::abs((y - top) / (bottom - top)) : 0;
    if (engine::MovieClip *bar = help->findChild("main_scrollpanel")) {
        if (engine::MovieClip *btn = bar->findChild("scroll_btn")) {
            setChildY(*bar, btn, "scroll_btn", helpScrollStart + ratio * helpScrollRange);
        }
    }
}

// AS: MainMenuScreen.releaseHelpMainArrow (stage MOUSE_UP) — QUIRK: it clears helpTabScrollDir, not
// helpMainScrollDir (the listener going away stops the scroll anyway).
void MainMenuScreen::releaseHelpMainArrow()
{
    helpTabScrollDir = 0;
    holdingMainArrow_ = false;
}

// AS: MainMenuScreen.clickHelpMainDragger
void MainMenuScreen::clickHelpMainDragger()
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    engine::MovieClip *btn = section("help")->findChild("main_scrollpanel")->findChild("scroll_btn");
    dragStartPointerY_ = gameObj->pointer.y;
    dragStartButtonY_ = btn->y();
    draggingMain_ = true;
}

// AS: MainMenuScreen.dragHelpMainDragger (ENTER_FRAME)
void MainMenuScreen::dragHelpMainDragger()
{
    engine::MovieClip *help = section("help");
    engine::MovieClip *btn_parent = help->findChild("main_scrollpanel");
    engine::MovieClip *btn = btn_parent->findChild("scroll_btn");
    engine::MovieClip *panel = help->findChild("mainpanel");
    engine::MovieClip *masker = help->findChild("mainmasker");
    const double by = std::clamp(dragStartButtonY_ + (gameObj->pointer.y - dragStartPointerY_), 30.0,
                                 30.0 + helpScrollRange);
    setChildY(*btn_parent, btn, "scroll_btn", by);
    const double ratio = (by - helpScrollStart) / helpScrollRange;
    if (panel && masker) {
        setChildY(*help, panel, "mainpanel",
                  childY(*help, masker, "mainmasker") - (clipHeight(*panel) - clipHeight(*masker)) * ratio);
    }
}

// AS: MainMenuScreen.releaseHelpMainDragger (stage MOUSE_UP)
void MainMenuScreen::releaseHelpMainDragger()
{
    draggingMain_ = false;
}

void MainMenuScreen::updateHelp()
{
    const bool up = !gameObj->pointer.down;
    if (holdingTabArrow_) {
        holdHelpTabArrow();
        if (up) {
            releaseHelpTabArrow();
        }
    }
    if (holdingMainArrow_) {
        holdHelpMainArrow();
        if (up) {
            releaseHelpMainArrow();
        }
    }
    if (draggingTab_) {
        dragHelpTabDragger();
        if (up) {
            releaseHelpTabDragger();
        }
    }
    if (draggingMain_) {
        dragHelpMainDragger();
        if (up) {
            releaseHelpMainDragger();
        }
    }
}

// AS: MainMenuScreen.setupMedals — the badge pages: earned / total, the first page, its slide-in. (Removing the
// listeners, setup false, is left to the clip going away.)
void MainMenuScreen::setupMedals(bool setup)
{
    const data::UserData &user = *gameObj->obf_51;
    const managers::ChallengeManager &challenges = *gameObj->obf_94;
    medalsPage = 0;
    medalsDirection = "next";
    medalsTransitioning = true;
    engine::MovieClip *medals = section("medals");
    if (!setup || !medals) {
        animatingMedals_ = false;
        return;
    }
    if (engine::MovieClip *next = medals->findChild("next_btn")) {
        next->onClick = [this] { clickNextMedals(); };
    }
    if (engine::MovieClip *prev = medals->findChild("prev_btn")) {
        prev->onClick = [this] { clickPrevMedals(); };
        prev->setVisible(false);
    }
    medals->setText("total_txt",
                    as3::String(user.getTotalBadgesEarned()) + "/" + as3::String(challenges.getNumberOfBadges()));
    populateMedals();
    animatingMedals_ = true;
}

// AS: MainMenuScreen.getDesaturatedFilter — the ColorMatrixFilter that turns a clip grey (luminance weights
// 0.212671, 0.71516, 0.072169): the engine's `desaturated` flag on the clip.

// AS: MainMenuScreen.populateMedals — this page's eight badges: title, description and tally, reward, picture and
// earned mark (an unearned one is greyed: getDesaturatedFilter); a locked one is the "Hidden Badge". Then the page arrows and number, and the slide-in.
void MainMenuScreen::populateMedals()
{
    const data::UserData &user = *gameObj->obf_51;
    const managers::ChallengeManager &challenges = *gameObj->obf_94;
    const engine::Lang &lang = *gameObj->lang;
    engine::MovieClip *medals = section("medals");
    engine::MovieClip *panel = medals ? medals->findChild("panel") : nullptr;
    if (!panel) {
        return;
    }
    const double first = 0 + medalsPage * medalsPerPage;
    const double last = std::min(challenges.getNumberOfBadges() - 1, first + (medalsPerPage - 1));
    const double count = last - first + 1;
    for (int i = 0; i < static_cast<int>(medalsPerPage); ++i) {
        engine::MovieClip *p = panel->findChild("panel" + std::to_string(i + 1));
        if (!p) {
            continue;
        }
        const double which = i + first;
        if (i < count) {
            p->setVisible(true);
            const double reward = challenges.getChallengeRewardAmount(-1, -1, which);
            p->setText("title_txt", challenges.getChallengeTitle(-1, -1, which));
            p->setText("description_txt",
                       challenges.getChallengeDescription(-1, -1, which) + " " + challenges.getBadgeTallyString(which));
            p->setText("reward_txt", lang.get("badge.reward", obf_AF::obf_81::obf_7w(reward)));
            p->setText("points_txt", lang.get("badge.points", as3::String(reward)));
            engine::MovieClip *thumb = p->findChild("thumb");
            if (thumb) {
                thumb->gotoAndStop(static_cast<int>(which) + 1);
            }
            if (engine::MovieClip *earned = p->findChild("earned")) {
                earned->setVisible(user.hasBadge(which));
            }
            if (thumb) {
                thumb->desaturated = !user.hasBadge(which); // thumb.filters = [getDesaturatedFilter()] or []
            }
            if (challenges.shouldLockBadge(which)) {
                if (thumb) {
                    thumb->gotoAndStop(49);
                }
                p->setText("title_txt", lang.get("badge.hidden_title"));
                p->setText("description_txt", lang.get("badge.hidden_text"));
                p->setText("reward_txt", "");
                p->setText("points_txt", "");
            }
        } else {
            p->setVisible(false);
        }
    }
    const double pages = std::ceil(challenges.getNumberOfBadges() / medalsPerPage);
    if (engine::MovieClip *prev = medals->findChild("prev_btn")) {
        prev->setVisible(medalsPage > 0);
    }
    if (engine::MovieClip *next = medals->findChild("next_btn")) {
        next->setVisible(medalsPage < pages - 1);
    }
    medals->setText("page_txt", as3::String(medalsPage + 1) + " / " + as3::String(pages));
    panel->gotoLabel(medalsDirection == "next" ? "innext" : "inprev", true);
}

// AS: MainMenuScreen.clickNextMedals
void MainMenuScreen::clickNextMedals()
{
    if (!medalsTransitioning) {
        ++medalsPage;
        medalsDirection = "next";
        medalsTransitioning = true;
        if (engine::MovieClip *panel = section("medals")->findChild("panel")) {
            panel->gotoLabel("outnext", true);
        }
        animatingMedals_ = true;
    }
}

// AS: MainMenuScreen.clickPrevMedals
void MainMenuScreen::clickPrevMedals()
{
    if (!medalsTransitioning) {
        --medalsPage;
        medalsDirection = "prev";
        medalsTransitioning = true;
        if (engine::MovieClip *panel = section("medals")->findChild("panel")) {
            panel->gotoLabel("outprev", true);
        }
        animatingMedals_ = true;
    }
}

// AS: MainMenuScreen.animateMedalsTransition — when the old page is out the new one is filled; when it is in, done.
void MainMenuScreen::animateMedalsTransition()
{
    engine::MovieClip *medals = section("medals");
    engine::MovieClip *panel = medals ? medals->findChild("panel") : nullptr;
    if (!panel) {
        animatingMedals_ = false;
        return;
    }
    const std::string label = panel->currentFrameLabel();
    if (label == "outnextframe" || label == "outprevframe") {
        populateMedals();
    } else if (label == "innextframe" || label == "inprevframe") {
        animatingMedals_ = false;
        medalsTransitioning = false;
    }
}

} // namespace papaGame::screens
