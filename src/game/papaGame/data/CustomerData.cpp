#include "game/papaGame/data/CustomerData.h"

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"
#include "game/papaGame/data/customers/CustomerAkari.h"
#include "game/papaGame/data/customers/CustomerBigPauly.h"
#include "game/papaGame/data/customers/CustomerBoomer.h"
#include "game/papaGame/data/customers/CustomerCaptainCori.h"
#include "game/papaGame/data/customers/CustomerClover.h"
#include "game/papaGame/data/customers/CustomerConnor.h"
#include "game/papaGame/data/customers/CustomerFoodini.h"
#include "game/papaGame/data/customers/CustomerGeorgito.h"
#include "game/papaGame/data/customers/CustomerGreg.h"
#include "game/papaGame/data/customers/CustomerJames.h"
#include "game/papaGame/data/customers/CustomerKahuna.h"
#include "game/papaGame/data/customers/CustomerKingsley.h"
#include "game/papaGame/data/customers/CustomerMarty.h"
#include "game/papaGame/data/customers/CustomerMindy.h"
#include "game/papaGame/data/customers/CustomerNinjoy.h"
#include "game/papaGame/data/customers/CustomerPapaLouie.h"
#include "game/papaGame/data/customers/CustomerPeggy.h"
#include "game/papaGame/data/customers/CustomerPenny.h"
#include "game/papaGame/data/customers/CustomerProfessorFitz.h"
#include "game/papaGame/data/customers/CustomerPrudence.h"
#include "game/papaGame/data/customers/CustomerRico.h"
#include "game/papaGame/data/customers/CustomerRita.h"
#include "game/papaGame/data/customers/CustomerSargeFan.h"
#include "game/papaGame/data/customers/CustomerScooter.h"
#include "game/papaGame/data/customers/CustomerTaylor.h"
#include "game/papaGame/data/customers/CustomerXandra.h"
#include "game/papaGame/data/customers/CustomerYippy.h"
#include "game/papaGame/data/customers/CustomerZoe.h"

namespace papaGame::data {

// AS: CustomerData.CustomerData
CustomerData::CustomerData(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    setupCustomerData();
}

// AS: CustomerData.setupCustomerData
void CustomerData::setupCustomerData()
{
    // customersIncluded, in the original's order (the index is the customer's id in the save data).
    customerDataFiles.clear();
    customerDataFiles.push_back(std::make_unique<customers::CustomerMarty>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerRita>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerPrudence>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerTaylor>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerClover>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerMindy>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerAkari>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerZoe>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerBigPauly>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerKahuna>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerKingsley>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerNinjoy>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerPenny>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerSargeFan>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerJames>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerScooter>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerConnor>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerPeggy>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerGeorgito>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerYippy>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerGreg>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerCaptainCori>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerRico>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerBoomer>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerProfessorFitz>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerFoodini>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerPapaLouie>());
    customerDataFiles.push_back(std::make_unique<customers::CustomerXandra>());
}

// AS: CustomerData.getCustomerClipName
std::string CustomerData::getCustomerClipName(double index) const
{
    const CustomerDataFile *c = getCustomerData(index);
    return c ? c->customerClipName : std::string();
}

// AS: CustomerData.getCustomerType
std::string CustomerData::getCustomerType(double index) const
{
    const CustomerDataFile *c = getCustomerData(index);
    return c ? c->weaponType : std::string();
}

// AS: CustomerData.getCustomerSkill
std::string CustomerData::getCustomerSkill(double index) const
{
    const CustomerDataFile *c = getCustomerData(index);
    return c ? c->skillType : std::string();
}

// AS: CustomerData.getWeaponClipName
std::string CustomerData::getWeaponClipName(double index, double style) const
{
    const CustomerDataFile *c = getCustomerData(index);
    if (!c) {
        return {};
    }
    std::string name = c->weaponClipName;
    if (c->weaponHasStyles && style > 1) {
        name += as3::String(style);
    }
    return name;
}

// AS: CustomerData.getCustomerData
const CustomerDataFile *CustomerData::getCustomerData(double index) const
{
    if (index >= 0 && index < static_cast<double>(customerDataFiles.size())) {
        return customerDataFiles[static_cast<size_t>(index)].get();
    }
    return nullptr;
}

// AS: CustomerData.getTrappedCustomerIndex — the customer caged in this world's cage `cageIndex`, or -1.
double CustomerData::getTrappedCustomerIndex(double whichWorld, double cageIndex) const
{
    for (size_t i = 0; i < customerDataFiles.size(); ++i) {
        const CustomerDataFile &c = *customerDataFiles[i];
        if (c.trappedWorld == whichWorld + 1 && c.trappedChar == cageIndex) {
            return static_cast<double>(i);
        }
    }
    return -1;
}

CustomerData::~CustomerData()
{
    if (thumbBMP) {
        SDL_DestroyTexture(thumbBMP);
    }
}

// AS: CustomerData.getCustomerName
std::string CustomerData::getCustomerName(double index) const
{
    const CustomerDataFile *c = getCustomerData(index);
    return c ? c->customerName : std::string();
}

// AS: CustomerData.generateCustomerBitmap
void CustomerData::generateCustomerBitmap()
{
    if (thumbBMP) {
        return;
    }
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    SDL_Renderer *renderer = gameObj->renderer;
    const int count = static_cast<int>(customerDataFiles.size());
    constexpr float kCell = 45;
    thumbBMP = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                 static_cast<int>(kCell * kThumbScale) * count, static_cast<int>(kCell * kThumbScale));
    if (!thumbBMP) {
        return;
    }
    SDL_SetTextureBlendMode(thumbBMP, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
    SDL_Texture *previous = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, thumbBMP);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    engine::RenderContext ctx{renderer, gameObj->symbols, gameObj->textures, gameObj->text};
    for (int i = 0; i < count; ++i) {
        std::unique_ptr<engine::MovieClip> model = engine::MovieClip::create(symbols, "customerOneSwingMC");
        if (!model) {
            continue;
        }
        const std::string name = getCustomerClipName(i);
        auto attach = [&](const char *slot, const std::string &part) {
            engine::MovieClip *holder = model->findChild(slot);
            std::unique_ptr<engine::MovieClip> clip = engine::MovieClip::create(symbols, "customer_" + name + "_" + part);
            if (holder && clip) {
                holder->addChild(std::move(clip), "clip");
            }
        };
        attach("body", "body");
        attach("head", "head");
        attach("eyes", "eyes");
        attach("mouth", "mouth");
        attach("neck", "neck");
        attach("hair", "hair");
        attach("back_hair", "back_hair");
        attach("front_upperarm", "upperarm");
        attach("back_upperarm", "upperarm");
        model->gotoLabel("stand", false);
        constexpr float kScale = 0.3f;
        // The head's own bounds put its left edge at -25 (in the model's space).
        float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
        engine::MovieClip *head = model->findChild("head");
        if (engine::MovieClip *part = head ? head->firstChildClip() : nullptr) {
            part->accumulateBounds(engine::Affine{}, minX, minY, maxX, maxY);
        }
        const float x = minX < maxX ? (-25 - minX) * kScale : 0;
        const SDL_Rect cell{static_cast<int>(static_cast<float>(i) * kCell * kThumbScale), 0,
                            static_cast<int>(kCell * kThumbScale), static_cast<int>(kCell * kThumbScale)};
        SDL_SetRenderClipRect(renderer, &cell);
        const engine::Affine m = engine::Affine::scale(kThumbScale) *
                                 engine::Affine::translate(static_cast<float>(i) * kCell, 0) *
                                 engine::Affine{kScale, 0, 0, kScale, x, 0};
        model->render(ctx, m, engine::ColorTransform{});
    }
    SDL_SetRenderClipRect(renderer, nullptr);
    SDL_SetRenderTarget(renderer, previous);
}

// AS: CustomerData.getCustomerBitmap (the rescued-customer row, param2, is never generated)
engine::MovieClip::BitmapChild CustomerData::getCustomerBitmap(double index) const
{
    engine::MovieClip::BitmapChild b;
    b.texture = thumbBMP;
    const float cell = 45 * kThumbScale;
    b.src = {static_cast<float>(index) * cell, 0, cell, cell};
    b.w = 45;
    b.h = 45;
    return b;
}

// AS: CustomerData.getCustomerIndex — by name or clip name; -1 when none.
double CustomerData::getCustomerIndex(const std::string &name) const
{
    double found = -1;
    for (size_t i = 0; i < customerDataFiles.size(); ++i) {
        if (customerDataFiles[i]->customerName == name || customerDataFiles[i]->customerClipName == name) {
            found = static_cast<double>(i);
            break;
        }
    }
    return found;
}

} // namespace papaGame::data
