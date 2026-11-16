#include "game/papaGame/screens/SlotSelectScreen.h"

#include <SDL3/SDL.h>

#include "engine/menu_bar.h"
#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"
#include "game/obf_AF/obf_81.h"

namespace papaGame::screens {

namespace {

// fl.controls.TextInput.restrict "0-9A-Za-z '\-" and maxChars 16.
bool allowedNameChar(char c)
{
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == ' ' || c == '\'' ||
           c == '-';
}

constexpr size_t kMaxNameChars = 16;

} // namespace

// AS: SlotSelectScreen.SlotSelectScreen
SlotSelectScreen::SlotSelectScreen(obf_9G::obf_4R *gameObj_, engine::MovieClip *container_)
    : gameObj(gameObj_), container(container_)
{
    setupScreen();
}

engine::MovieClip *SlotSelectScreen::slotClip(int slot)
{
    engine::MovieClip *slots = clip->findChild("slots");
    return slots ? slots->findChild("slot" + std::to_string(slot) + "MC") : nullptr;
}

// AS: SlotSelectScreen.setupScreen
void SlotSelectScreen::setupScreen()
{
    clip = &container->addChild(engine::MovieClip::create(*gameObj->symbols, "slotSelectMC"), "");
    for (int i = 1; i <= 3; ++i) {
        engine::MovieClip *slot = slotClip(i);
        if (!slot) {
            continue;
        }
        const double n = i;
        if (engine::MovieClip *b = slot->findChild("new_btn")) {
            b->onClick = [this, n] { clickNewSlot(n); };
        }
        if (engine::MovieClip *b = slot->findChild("select_btn")) {
            b->onClick = [this, n] { clickExistingSlot(n); };
        }
        if (engine::MovieClip *b = slot->findChild("delete_btn")) {
            b->onClick = [this, n] { clickDeleteSlot(n); };
        }
        if (engine::MovieClip *confirm = slot->findChild("confirmMC")) {
            if (engine::MovieClip *b = confirm->findChild("yes_btn")) {
                b->onClick = [this, n] { clickDeleteYes(n); };
            }
            if (engine::MovieClip *b = confirm->findChild("no_btn")) {
                b->onClick = [this] { clickDeleteNo(); };
            }
            confirm->setVisible(false);
        }
        // The backup buttons used the online API: never shown (the original hides them off its sites too).
        for (const char *backup : {"savebackup_btn", "loadbackup_btn"}) {
            if (engine::MovieClip *b = slot->findChild(backup)) {
                b->setVisible(false);
            }
        }
    }
    engine::MovieClip *entername = clip->findChild("entername");
    engine::MovieClip *holder = entername ? entername->findChild("input_holder") : nullptr;
    if (holder) {
        auto back = engine::makeApiButton(*gameObj->symbols, gameObj->text, gameObj->lang->get("menu.name_back"),
                                          false, 112);
        back->setPosition(0, 75);
        back->onClick = [this] { clickEnterNameCancel(); };
        holder->addChild(std::move(back), "nameBackButton");
        auto next = engine::makeApiButton(*gameObj->symbols, gameObj->text, gameObj->lang->get("menu.name_continue"),
                                          false, 112);
        next->setPosition(181, 75);
        next->onClick = [this] { clickEnterNameOK(); };
        holder->addChild(std::move(next), "nameContinueButton");
        // The TextInput component: on the timeline it is only its avatar and skins; at run time it draws its up
        // skin at its size (the 100 x 22 avatar scaled 2.94 x 1.77) with the text (Arial bold 24, centred).
        if (engine::MovieClip *input = holder->findChild("input_txt")) {
            input->setVisible(false);
        }
        constexpr float kInputX = 0, kInputY = 25, kInputW = 294, kInputH = 39;
        if (auto skin = engine::MovieClip::create(*gameObj->symbols, "TextInput_upSkin")) {
            float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
            skin->accumulateBounds(engine::Affine{}, minX, minY, maxX, maxY);
            if (maxX > minX && maxY > minY) {
                skin->setScale(kInputW / (maxX - minX), kInputH / (maxY - minY));
            }
            skin->setPosition(kInputX, kInputY);
            skin->mouseEnabled = false;
            holder->addChild(std::move(skin), "inputSkin");
        }
        engine::MovieClip::TextField field;
        field.font = "Arial Bold";
        field.size = 24;
        field.x = kInputX;
        field.y = kInputY + 4;
        field.width = kInputW;
        field.align = 2;
        holder->addTextField("input_txt", field);
    }
    gameObj->addContainerListener("clickBack", [this] { clickBack(); });
    if (entername) {
        entername->gotoAndStop(1);
    }
    if (engine::MovieClip *slots = clip->findChild("slots")) {
        slots->gotoAndStop(1);
    }
    engine::MovieClip *character = clip->findChild("character");
    if (character) {
        character->gotoAndStop(1);
        if (engine::MovieClip *marty = character->findChild("marty")) {
            marty->setText("name_txt", "Marty");
            if (engine::MovieClip *b = marty->findChild("select_btn")) {
                b->onMouseDown = [this] { clickMarty(); };
            }
        }
        if (engine::MovieClip *rita = character->findChild("rita")) {
            rita->setText("name_txt", "Rita");
            if (engine::MovieClip *b = rita->findChild("select_btn")) {
                b->onMouseDown = [this] { clickRita(); };
            }
        }
    }
    if (engine::MovieClip *missing = clip->findChild("missing")) {
        missing->setVisible(false); // the "missing save" widget: an API feature
    }
    showSlots();
}

// AS: SlotSelectScreen.keyListener — Enter confirms the name. Port: the keyboard's text and backspace too.
void SlotSelectScreen::keyListener(int key)
{
    if (currentMode != "entername") {
        return;
    }
    if (key == SDLK_RETURN || key == SDLK_KP_ENTER) {
        clickEnterNameOK();
    } else if (key == SDLK_BACKSPACE && !inputText_.empty()) {
        inputText_.pop_back();
        setInputText(inputText_);
    }
}

void SlotSelectScreen::setInputText(const std::string &text)
{
    inputText_ = text;
    engine::MovieClip *entername = clip->findChild("entername");
    engine::MovieClip *holder = entername ? entername->findChild("input_holder") : nullptr;
    if (holder) {
        if (engine::MovieClip::TextField *field = holder->textField("input_txt")) {
            const bool caret = currentMode == "entername" && (caretTimer_ / 15) % 2 == 0;
            field->text = text + (caret ? "|" : "");
        }
    }
}

// AS: SlotSelectScreen.clickBack — back to the title screen (the ad call is dropped).
void SlotSelectScreen::clickBack()
{
    gameObj->showApiScreen("SplashScreen");
}

// AS: SlotSelectScreen.getSlotData — each slot's name, puppet and totals, or "Empty Slot".
void SlotSelectScreen::getSlotData()
{
    for (int i = 1; i <= 3; ++i) {
        std::unique_ptr<data::SlotLabels> labels = gameObj->obf_51->loadLabelsForSlot(i);
        engine::MovieClip *slot = slotClip(i);
        if (!slot) {
            continue;
        }
        engine::MovieClip *textclip = slot->findChild("textclip");
        engine::MovieClip **model = i == 1 ? &model1 : (i == 2 ? &model2 : &model3);
        auto show = [&](const char *name, bool visible) {
            if (engine::MovieClip *b = slot->findChild(name)) {
                b->setVisible(visible);
            }
        };
        if (labels) {
            slot->setText("name_txt", labels->name);
            if (textclip) {
                textclip->setVisible(true);
                if (*model) {
                    textclip->removeChild(*model);
                    *model = nullptr;
                }
                if (auto m = buildModel(labels->character, labels->style)) {
                    m->setPosition(10, -60);
                    *model = &textclip->addChild(std::move(m), "");
                }
                textclip->setText("points_txt", obf_AF::obf_81::obf_7w(labels->score));
                textclip->setText("coins_txt", obf_AF::obf_81::obf_7w(labels->coins));
                textclip->setText("time_txt", obf_AF::obf_81::obf_2i(labels->time, false, true));
                textclip->setText("warpcoins_txt", as3::String(labels->warpcoins) + "/50");
                textclip->setText("customers_txt", as3::String(labels->customers) + "/28");
                textclip->setText("percent_txt", as3::String(as3::Number(labels->completion)));
                if (engine::MovieClip *star = textclip->findChild("star")) {
                    star->setVisible(as3::Number(labels->completion) == 100);
                }
            }
            show("select_btn", true);
            show("new_btn", false);
            show("delete_btn", true);
        } else {
            slot->setText("name_txt", gameObj->lang->get("slot.empty"));
            if (textclip) {
                textclip->setVisible(false);
            }
            show("new_btn", true);
            show("select_btn", false);
            show("delete_btn", false);
        }
    }
}

// AS: SlotSelectScreen.clickNewSlot
void SlotSelectScreen::clickNewSlot(double slot)
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    if (engine::MovieClip *slots = clip->findChild("slots")) {
        slots->mouseEnabled = false;
        slots->mouseChildren = false;
    }
    selectedSlot = slot;
    isNewSlot = true;
    showCharacter();
}

// AS: SlotSelectScreen.clickExistingSlot
void SlotSelectScreen::clickExistingSlot(double slot)
{
    gameObj->obf_51->loadData(slot);
    isNewSlot = false;
    if (engine::MovieClip *slots = clip->findChild("slots")) {
        slots->gotoLabel("hide", true);
        slots->mouseEnabled = false;
        slots->mouseChildren = false;
    }
    startClosingScreen();
}

// AS: SlotSelectScreen.clickDeleteSlot — asks first.
void SlotSelectScreen::clickDeleteSlot(double slot)
{
    if (engine::MovieClip *s = slotClip(static_cast<int>(slot))) {
        if (engine::MovieClip *confirm = s->findChild("confirmMC")) {
            confirm->setVisible(true);
        }
    }
}

// AS: SlotSelectScreen.clickDeleteYes
void SlotSelectScreen::clickDeleteYes(double slot)
{
    if (slot > 0) {
        gameObj->obf_51->eraseSlot(slot);
        showSlots();
    }
}

// AS: SlotSelectScreen.clickDeleteNo
void SlotSelectScreen::clickDeleteNo()
{
    for (int i = 1; i <= 3; ++i) {
        if (engine::MovieClip *s = slotClip(i)) {
            if (engine::MovieClip *confirm = s->findChild("confirmMC")) {
                confirm->setVisible(false);
            }
        }
    }
    gameObj->obf_7C->playSound("buttonclick.wav");
}

// AS: SlotSelectScreen.clickEnterNameOK — a blank name becomes "Player <slot>".
void SlotSelectScreen::clickEnterNameOK()
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    std::string name = inputText_;
    if (name.empty() || name == " " || name == "  " || name == "   ") {
        name = gameObj->lang->get("slot.player") + " " + as3::String(selectedSlot);
    }
    gameObj->onTextInput = nullptr; // stage.focus = stage
    gameObj->onKeyDown = nullptr;
    gameObj->obf_51->createNewSlot(selectedSlot, name, selectedCharacter);
    if (engine::MovieClip *entername = clip->findChild("entername")) {
        entername->gotoLabel("hide", true);
    }
    startClosingScreen();
}

// AS: SlotSelectScreen.clickEnterNameCancel
void SlotSelectScreen::clickEnterNameCancel()
{
    gameObj->obf_7C->playSound("buttonclick.wav");
    showSlots();
}

// AS: SlotSelectScreen.clickMarty
void SlotSelectScreen::clickMarty()
{
    selectedCharacter = "marty";
    if (engine::MovieClip *character = clip->findChild("character")) {
        character->gotoLabel("hide", true);
    }
    showEnterName();
}

// AS: SlotSelectScreen.clickRita
void SlotSelectScreen::clickRita()
{
    selectedCharacter = "rita";
    if (engine::MovieClip *character = clip->findChild("character")) {
        character->gotoLabel("hide", true);
    }
    showEnterName();
}

// AS: SlotSelectScreen.showSlots
void SlotSelectScreen::showSlots()
{
    currentMode = "slots";
    gameObj->onTextInput = nullptr;
    gameObj->onKeyDown = nullptr;
    gameObj->menuBar.setTitle(gameObj->lang, "title.choose_slot");
    engine::MovieClip *slots = clip->findChild("slots");
    if (slots) {
        slots->mouseEnabled = true;
        slots->mouseChildren = true;
    }
    for (int i = 1; i <= 3; ++i) {
        if (engine::MovieClip *s = slotClip(i)) {
            if (engine::MovieClip *confirm = s->findChild("confirmMC")) {
                confirm->setVisible(false);
            }
        }
    }
    getSlotData();
    engine::MovieClip *entername = clip->findChild("entername");
    if (entername && entername->currentFrame() + 1 > 1 && entername->currentFrame() + 1 < entername->frameCount()) {
        entername->gotoLabel("hide", true);
    }
    gameObj->obf_7C->playSound("trayslide.wav");
    if (slots) {
        slots->gotoLabel("show", true);
    }
}

// AS: SlotSelectScreen.showEnterName — the name field gets the keyboard (on a phone, the on-screen one).
void SlotSelectScreen::showEnterName()
{
    currentMode = "entername";
    setInputText("");
    gameObj->onTextInput = [this](const std::string &typed) {
        std::string text = inputText_;
        for (char c : typed) {
            if (allowedNameChar(c) && text.size() < kMaxNameChars) {
                text += c;
            }
        }
        setInputText(text);
    };
    gameObj->onKeyDown = [this](int key) { keyListener(key); };
    engine::MovieClip *character = clip->findChild("character");
    if (character && character->currentFrame() + 1 > 1 && character->currentFrame() + 1 < character->frameCount()) {
        character->gotoLabel("hide", true);
    }
    if (engine::MovieClip *entername = clip->findChild("entername")) {
        entername->gotoLabel("show", true);
    }
    gameObj->menuBar.setTitle(gameObj->lang, "title.enter_name");
}

// AS: SlotSelectScreen.showCharacter — Marty and Rita to choose from.
void SlotSelectScreen::showCharacter()
{
    currentMode = "character";
    engine::MovieClip *slots = clip->findChild("slots");
    if (slots && slots->currentFrame() + 1 > 1 && slots->currentFrame() + 1 < slots->frameCount()) {
        slots->gotoLabel("hide", true);
    }
    engine::MovieClip *character = clip->findChild("character");
    if (character) {
        engine::MovieClip *marty = character->findChild("marty");
        engine::MovieClip *martyHolder = marty ? marty->findChild("holder") : nullptr;
        if (!martyModel && martyHolder) {
            if (auto m = buildModel(0, 1)) {
                m->setPosition(-30, -85);
                m->setScale(0.48f, 0.48f);
                martyModel = &martyHolder->addChild(std::move(m), "");
            }
            martyHolder->setScale(-1, 1);
        }
        engine::MovieClip *rita = character->findChild("rita");
        engine::MovieClip *ritaHolder = rita ? rita->findChild("holder") : nullptr;
        if (!ritaModel && ritaHolder) {
            if (auto m = buildModel(1, 1)) {
                m->setPosition(-30, -85);
                m->setScale(0.48f, 0.48f);
                ritaModel = &ritaHolder->addChild(std::move(m), "");
            }
        }
        character->gotoLabel("show", true);
    }
    gameObj->menuBar.setTitle(gameObj->lang, "title.choose_character");
}

// AS: SlotSelectScreen.animateScreen (ENTER_FRAME) — closes once the tray has slid away.
void SlotSelectScreen::animateScreen()
{
    if (currentMode == "entername") {
        ++caretTimer_;
        setInputText(inputText_);
    }
    if (isClosing) {
        engine::MovieClip *entername = clip->findChild("entername");
        engine::MovieClip *slots = clip->findChild("slots");
        if (currentMode == "entername" && entername && entername->currentFrame() + 1 == entername->frameCount()) {
            closeSlotSelectScreen();
        } else if (currentMode == "slots" && slots && slots->currentFrame() + 1 == slots->frameCount()) {
            closeSlotSelectScreen();
        }
    }
}

// AS: SlotSelectScreen.destroy
void SlotSelectScreen::destroy()
{
    gameObj->removeContainerListener("clickBack");
    gameObj->onTextInput = nullptr;
    gameObj->onKeyDown = nullptr;
    container->removeChild(clip);
    clip = nullptr;
}

// AS: SlotSelectScreen.startClosingScreen (the API and ad calls are dropped)
void SlotSelectScreen::startClosingScreen()
{
    isClosing = true;
}

// AS: SlotSelectScreen.closeSlotSelectScreen — a new game starts with the story, a loaded one at the map.
// SKELETON: CustomerData.generateCustomerBitmap and DataManager.generateEnemyBitmap (the menus' thumbnails).
void SlotSelectScreen::closeSlotSelectScreen()
{
    isClosing = false;
    if (isNewSlot) {
        gameObj->obf_19();
        gameObj->showApiScreen("");
    } else {
        gameObj->showApiScreen("MainMenu");
    }
}

// AS: SlotSelectScreen.buildModel — the customer's puppet for its weapon, dressed in its parts, standing.
std::unique_ptr<engine::MovieClip> SlotSelectScreen::buildModel(double whichCharacter, double style)
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    const std::string type = gameObj->obf_3A->getCustomerType(whichCharacter);
    using data::CustomerData;
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
        return nullptr;
    }
    model->setScale(0.35f, 0.35f);
    const std::string clipName = gameObj->obf_3A->getCustomerClipName(whichCharacter);
    std::string name = clipName;
    if (style == 2) {
        name += "2";
    } else if (style == 3) {
        name += "3";
    }
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
    const std::string weapon = "weapon_" + gameObj->obf_3A->getWeaponClipName(whichCharacter, style);
    attach("weapon", weapon);
    attach("cross_back_weap", weapon);
    if (clipName == "Boomer") {
        attach("glider", "glider_" + clipName);
    } else if (engine::MovieClip *glider = model->findChild("glider")) {
        glider->addChild(engine::MovieClip::createEmpty(symbols), "clip");
    }
    model->gotoAndStop(1);
    model->gotoLabel("stand", false);
    if (clipName == "Connor") {
        model->gotoLabel("standconnor", false);
    }
    model->mouseEnabled = false;
    model->mouseChildren = false;
    return model;
}

} // namespace papaGame::screens
