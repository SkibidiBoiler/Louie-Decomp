#include "game/papaGame/managers/CharacterBlitter.h"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cmath>

#include "engine/textures.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::managers {

namespace {

using Frames = std::vector<double>;

const Frames basicCycles_onehand = {2, 6, 10, 14, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 36, 37, 38, 43, 44, 45, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100};
const Frames basicCycles_fat_onehand = {2, 6, 10, 14, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 36, 37, 38, 43, 44, 45, 47, 48, 49, 50, 51, 52, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100};
const Frames basicCycles_crawler_onehand = {2, 6, 10, 14, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 36, 37, 38, 43, 44, 45, 47, 48, 49, 50, 51, 52, 230, 231, 232, 233, 234, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100};
const Frames basicCycles_connor_onehand = {281, 285, 289, 293, 298, 299, 300, 301, 302, 303, 304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 36, 37, 38, 43, 44, 45, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100};
const Frames attackCycles_onehand = {102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127};
const Frames attackCycles_ninjoy = {289, 290, 291, 296, 309, 310, 311, 317, 328, 329, 330, 331, 336, 346, 347, 348, 353, 2, 2, 2, 2, 2, 2, 2, 2};
const Frames attackCycles_clover = {387, 388, 389, 395, 418, 419, 420, 432, 441, 442, 443, 444, 451, 464, 465, 466, 471, 2, 2, 2, 2, 2, 2, 2, 2};
const Frames grabendCycles_onehand = {141, 142, 143, 144, 145, 146, 148, 149};
const Frames pound_onehand = {161, 162, 163};
const Frames doublejump_onehand = {177, 178, 179, 180, 181, 182};
const Frames glide_onehand = {197, 198, 199, 200, 201, 202, 203};
const Frames wallslide_onehand = {217, 218, 219, 220};
const Frames crawl_onehand = {235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 252, 253, 254, 255, 256, 257, 258};
const Frames push_onehand = {260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 270, 271, 272, 273, 274, 275, 277, 278, 279};
const Frames slide_scooter = {285, 286, 287, 288, 289, 290};
const Frames cagedCycles = {2, 6, 10, 14, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};

void concat(Frames &a, const Frames &b) { a.insert(a.end(), b.begin(), b.end()); }

// A MovieClip with an empty one-frame timeline (AS: new MovieClip()).
const engine::Timeline &emptyTimeline()
{
    static const engine::Timeline tl{};
    return tl;
}

} // namespace

// AS: CharacterBlitter.CharacterBlitter
CharacterBlitter::CharacterBlitter(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
}

CharacterBlitter::~CharacterBlitter()
{
    destroy();
}

// AS: CharacterBlitter.createPlayerSprite
void CharacterBlitter::createPlayerSprite(const std::function<void()> &callback_, bool isCaged_,
                                          const std::string &cagedName)
{
    SDL_Log("Start Creating Player Sprite");
    isCaged = isCaged_;
    whichCharacter = gameObj->obf_3A->getCustomerClipName(gameObj->obf_51->selectedCharacter);
    whichStyle = gameObj->obf_51->selectedStyle;
    if (isCaged_) {
        whichCharacter = cagedName;
        whichStyle = 1;
    }
    callback = callback_;
    buildModel();
    startBlittingPhase();
    // ENTER_FRAME on models[0] runs blitFrame until the last phase finishes.
    finished_ = false;
    for (int guard = 0; !finished_ && guard < 10000; ++guard) {
        blitFrame();
    }
}

// AS: CharacterBlitter.setupFramesNeeded
void CharacterBlitter::setupFramesNeeded()
{
    const data::CustomerDataFile *c = gameObj->obf_3A->getCustomerData(gameObj->obf_51->selectedCharacter);
    if (!c) {
        framesNeeded.clear();
        return;
    }
    if (c->isHuge) {
        framesNeeded = basicCycles_fat_onehand;
    } else if (c->skillType == data::CustomerData::SKILL_CRAWL) {
        framesNeeded = basicCycles_crawler_onehand;
    } else if (c->customerName == "Connor") {
        framesNeeded = basicCycles_connor_onehand;
    } else {
        framesNeeded = basicCycles_onehand;
    }
    if (c->customerName == "Ninjoy") {
        concat(framesNeeded, attackCycles_ninjoy);
    } else if (c->customerName == "Clover") {
        concat(framesNeeded, attackCycles_clover);
    } else {
        concat(framesNeeded, attackCycles_onehand);
    }
    concat(framesNeeded, grabendCycles_onehand);
    if (c->skillType == data::CustomerData::SKILL_POUND) {
        concat(framesNeeded, pound_onehand);
    } else if (c->skillType == data::CustomerData::SKILL_DOUBLEJUMP) {
        concat(framesNeeded, doublejump_onehand);
    } else if (c->skillType == data::CustomerData::SKILL_GLIDE) {
        concat(framesNeeded, glide_onehand);
    } else if (c->skillType == data::CustomerData::SKILL_WALLJUMP) {
        concat(framesNeeded, wallslide_onehand);
    } else if (c->skillType == data::CustomerData::SKILL_CRAWL) {
        concat(framesNeeded, crawl_onehand);
    } else if (c->skillType == data::CustomerData::SKILL_PUSH) {
        concat(framesNeeded, push_onehand);
    }
    if (c->customerName == "Scooter") {
        concat(framesNeeded, slide_scooter);
    }
    if (isCaged) {
        framesNeeded = cagedCycles;
    }
}

// Port: a render target holding a sheet of `width` x `height` sheet units, at the shape texture scale.
engine::Sheet CharacterBlitter::createSheet(double width, double height)
{
    SDL_Renderer *renderer = gameObj->renderer;
    float scale = static_cast<float>(gameObj->textures->scale());
    const int maxSize = static_cast<int>(SDL_GetNumberProperty(SDL_GetRendererProperties(renderer),
                                                               SDL_PROP_RENDERER_MAX_TEXTURE_SIZE_NUMBER, 8192));
    while (scale > 1.0f && (width * scale > maxSize || height * scale > maxSize)) {
        scale -= 1.0f;
    }
    engine::Sheet sheet;
    sheet.scale = scale;
    sheet.width = static_cast<float>(width);
    sheet.height = static_cast<float>(height);
    sheet.premultiplied = true;
    sheet.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                      std::max(1, static_cast<int>(std::ceil(width * scale))),
                                      std::max(1, static_cast<int>(std::ceil(height * scale))));
    if (!sheet.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Character sheet %gx%g: %s", width, height, SDL_GetError());
        return sheet;
    }
    SDL_SetTextureBlendMode(sheet.texture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
    SDL_SetTextureScaleMode(sheet.texture, SDL_SCALEMODE_LINEAR);
    SDL_Texture *previous = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, sheet.texture);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderTarget(renderer, previous);
    return sheet;
}

// BitmapData.draw(model, matrix, null, null, null, true). draw() ignores the source's own transform and
// filters, so the models' scale and GlowFilter do not reach the sheet.
void CharacterBlitter::drawModel(engine::Sheet &sheet, engine::MovieClip &model, const engine::Affine &matrix)
{
    if (!sheet.texture) {
        return;
    }
    SDL_Renderer *renderer = gameObj->renderer;
    SDL_Texture *previous = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, sheet.texture);
    engine::RenderContext ctx{renderer, gameObj->symbols, gameObj->textures, gameObj->text};
    model.render(ctx, engine::Affine::scale(sheet.scale) * matrix, engine::ColorTransform{});
    SDL_SetRenderTarget(renderer, previous);
}

// AS: CharacterBlitter.startBlittingPhase
void CharacterBlitter::startBlittingPhase()
{
    startAnimFrame = 1;
    currentAnimFrame = startAnimFrame - numModels;
    setupFramesNeeded();
    totalAnimFrames = static_cast<double>(framesNeeded.size());
    setupModelForPhase();
    if (models.empty() || totalAnimFrames <= 0) {
        finished_ = true;
        return;
    }
    engine::MovieClip &model = *models[0];
    const char *boundsName = whichBlitPhase == "front_shoe" || whichBlitPhase == "back_shoe" ? "foot_bounds"
                                                                                             : "torso_bounds";
    // x/width of a named child in the model's space (times the model's scale, |scaleX| = modelScale).
    auto childBox = [&](const char *name, double &x, double &y, double &w, double &h) {
        engine::Affine m;
        x = y = w = h = 0;
        if (!model.childMatrix(name, m)) {
            return;
        }
        x = m.tx;
        y = m.ty;
        if (engine::MovieClip *c = model.findChild(name)) {
            float x0 = 1e30f, y0 = 1e30f, x1 = -1e30f, y1 = -1e30f;
            c->accumulateBounds(m, x0, y0, x1, y1);
            if (x1 > x0) {
                w = x1 - x0;
                h = y1 - y0;
            }
        }
    };
    double bx, by, bw, bh;
    childBox(boundsName, bx, by, bw, bh);
    boundsWidth = std::ceil(bw * modelScale);
    boundsHeight = std::ceil(bh * modelScale);
    boundsX = std::floor(bx * modelScale);
    boundsY = std::floor(by * modelScale);
    if (boundsWidth <= 0 || boundsHeight <= 0) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CharacterBlitter: no %s in the puppet", boundsName);
        finished_ = true;
        return;
    }
    blitSpritesPerRow = std::floor(maxSheetPxWidth / boundsWidth);
    double rows = std::ceil(totalAnimFrames / blitSpritesPerRow);
    if (std::ceil(totalAnimFrames / rows) < blitSpritesPerRow) {
        blitSpritesPerRow = std::ceil(totalAnimFrames / rows);
    }
    if (blitFlipped) {
        rows *= 2;
    }
    double tx, ty, tw, th, fx, fy, fw, fh, cx, cy, cw, ch;
    childBox("torso_bounds", tx, ty, tw, th);
    childBox("foot_bounds", fx, fy, fw, fh);
    childBox("center_point", cx, cy, cw, ch);
    if (whichBlitPhase == "torso") {
        torsoSheet = createSheet(boundsWidth * blitSpritesPerRow, boundsHeight * rows);
        spritesPerRow = blitSpritesPerRow;
        spriteWidth = boundsWidth;
        spriteHeight = boundsHeight;
        flipBlitOffset = std::ceil(totalAnimFrames / blitSpritesPerRow) * boundsHeight;
        torsoOffsetX = std::floor(tx * modelScale) - std::floor(fx * modelScale);
        torsoOffsetFlippedX = std::floor((tx + tw) * modelScale) - std::floor((fx + fw) * modelScale);
        torsoOffsetY = std::floor(ty * modelScale) - std::floor(fy * modelScale);
    } else if (whichBlitPhase == "front_shoe") {
        frontFootSheet = createSheet(boundsWidth * blitSpritesPerRow, boundsHeight * rows);
        footSpritesPerRow = blitSpritesPerRow;
        footSpriteWidth = boundsWidth;
        footSpriteHeight = boundsHeight;
        footFlipBlitOffset = std::ceil(totalAnimFrames / blitSpritesPerRow) * boundsHeight;
        spriteCenterX = std::ceil(footSpriteWidth / 2);
        spriteCenterY = std::ceil(footSpriteHeight / 2);
        spriteTargetX = spriteCenterX;
        spriteTargetY = spriteCenterY;
        blitOffsetX = std::round(cx * modelScale - boundsX);
        blitFlippedOffsetX = std::round(boundsX + boundsWidth - cx * modelScale);
    } else if (whichBlitPhase == "back_shoe") {
        backFootSheet = createSheet(boundsWidth * blitSpritesPerRow, boundsHeight * rows);
    }
    const auto s = static_cast<float>(modelScale);
    blitMatrix = engine::Affine{s, 0, 0, s, 0, 0};
    blitMatrixFlipped = engine::Affine{-s, 0, 0, s, 0, 0};
    for (auto &m : models) {
        m->gotoAndStop(1);
    }
}

// AS: CharacterBlitter.blitFrame
void CharacterBlitter::blitFrame()
{
    engine::Sheet *sheet = whichBlitPhase == "torso"        ? &torsoSheet
                           : whichBlitPhase == "front_shoe" ? &frontFootSheet
                                                            : &backFootSheet;
    for (int i = 0; i < static_cast<int>(numModels) && i < static_cast<int>(models.size()); ++i) {
        if (currentAnimFrame > 0 && currentAnimFrame + i <= totalAnimFrames) {
            const double row = std::floor((currentAnimFrame - 1 + i) / blitSpritesPerRow);
            const double col = currentAnimFrame - 1 + i - row * blitSpritesPerRow;
            double flipOffset = 0;
            if (blitFlipped) {
                flipOffset = std::ceil(totalAnimFrames / blitSpritesPerRow) * boundsHeight;
                // Flipped frames go in the top half, unflipped ones below them.
                blitMatrixFlipped.tx = static_cast<float>(col * boundsWidth + boundsX + boundsWidth);
                blitMatrixFlipped.ty = static_cast<float>(0 - boundsY + row * boundsHeight);
                drawModel(*sheet, *models[static_cast<size_t>(i)], blitMatrixFlipped);
            }
            blitMatrix.tx = static_cast<float>(col * boundsWidth - boundsX);
            blitMatrix.ty = static_cast<float>(0 - boundsY + row * boundsHeight + flipOffset);
            drawModel(*sheet, *models[static_cast<size_t>(i)], blitMatrix);
        }
    }
    if (currentAnimFrame >= totalAnimFrames) {
        finishBlittingPhase();
        return;
    }
    currentAnimFrame += numModels;
    for (size_t i = 0; i < models.size(); ++i) {
        const double base = currentAnimFrame - numModels + static_cast<double>(i);
        // QUIRK: every model is sent to five frames in a row; each gotoAndStop runs that frame's script and
        // the last one wins. (Past the end of framesNeeded the original throws; those frames are not drawn.)
        for (int j = 0; j < static_cast<int>(numModels); ++j) {
            const double k = base + j;
            if (k >= 0 && k < static_cast<double>(framesNeeded.size())) {
                models[i]->gotoAndStop(static_cast<int>(framesNeeded[static_cast<size_t>(k)]));
            }
        }
    }
}

// AS: CharacterBlitter.finishBlittingPhase
void CharacterBlitter::finishBlittingPhase()
{
    if (whichBlitPhase == "torso") {
        if (isCaged) {
            finishBlitting();
        } else {
            whichBlitPhase = "front_shoe";
            startBlittingPhase();
        }
    } else if (whichBlitPhase == "front_shoe") {
        whichBlitPhase = "back_shoe";
        startBlittingPhase();
    } else {
        finishBlitting();
    }
}

// AS: CharacterBlitter.finishBlitting
void CharacterBlitter::finishBlitting()
{
    SDL_Log("Finish Blitting");
    finished_ = true;
    cleanupModel();
    if (callback) {
        callback();
    }
    callback = nullptr;
}

// AS: CharacterBlitter.setupModelForPhase
void CharacterBlitter::setupModelForPhase()
{
    for (auto &model : models) {
        model->forEachChild([&](const std::string &name, engine::MovieClip &child) {
            if (isCaged && whichBlitPhase == "torso" && name == "foot_bounds") {
                child.setVisible(false);
            } else if (!isCaged && whichBlitPhase == "torso" &&
                       (name == "front_shoe" || name == "back_shoe" || name == "foot_bounds")) {
                child.setVisible(false);
            } else if (whichBlitPhase == "front_shoe" && name != "front_shoe" && name != "foot_bounds") {
                child.setVisible(false);
            } else if (whichBlitPhase == "back_shoe" && name != "back_shoe" && name != "foot_bounds") {
                child.setVisible(false);
            } else {
                child.setVisible(true);
            }
        });
    }
}

// AS: CharacterBlitter.buildModel
void CharacterBlitter::buildModel()
{
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    auto make = [&](const std::string &cls) -> std::unique_ptr<engine::MovieClip> {
        const int id = symbols.findClass(cls);
        const engine::Timeline *tl = id >= 0 ? symbols.timeline(static_cast<uint16_t>(id)) : nullptr;
        return tl ? std::make_unique<engine::MovieClip>(symbols, *tl) : nullptr;
    };
    // container.addChild(new <cls>()) named "clip"; a missing class throws in the original (the try blocks
    // around hair, back_hair, cross_* and weapon swallow it).
    auto dress = [&](engine::MovieClip &model, const char *container, const std::string &cls) {
        engine::MovieClip *target = model.findChild(container);
        auto part = make(cls);
        if (target && part) {
            target->addChild(std::move(part), "clip");
        }
    };
    models.clear();
    const data::CustomerData &customers = *gameObj->obf_3A;
    const double selected = gameObj->obf_51->selectedCharacter;
    for (int i = 0; i < static_cast<int>(numModels); ++i) {
        const std::string type = customers.getCustomerType(selected);
        std::string modelClass = "customerOneSwingMC";
        if (isCaged) modelClass = "customerTrappedMC";
        else if (type == data::CustomerData::WEAPON_SWING1) modelClass = "customerOneSwingMC";
        else if (type == data::CustomerData::WEAPON_SWING2) modelClass = "customerTwoSwingMC";
        else if (type == data::CustomerData::WEAPON_SCOOTER) modelClass = "customerScooterMC";
        else if (type == data::CustomerData::WEAPON_KAHUNA) modelClass = "customerKahunaMC";
        else if (type == data::CustomerData::WEAPON_WHIP) modelClass = "customerWhipMC";
        else if (type == data::CustomerData::WEAPON_MELEE) modelClass = "customerMeleeMC";
        else if (type == data::CustomerData::WEAPON_TOSS) modelClass = "customerTossMC";
        else if (type == data::CustomerData::WEAPON_LONGGUN) modelClass = "customerLongGunMC";
        else if (type == data::CustomerData::WEAPON_PISTOL) modelClass = "customerPistolMC";
        else if (type == data::CustomerData::WEAPON_BAZOOKA) modelClass = "customerBazookaMC";
        auto model = make(modelClass);
        if (!model) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "CharacterBlitter: no %s", modelClass.c_str());
            return;
        }
        std::string name = whichCharacter;
        if (whichStyle == 2) {
            name += "2";
        } else if (whichStyle == 3) {
            name += "3";
        }
        const std::string prefix = "customer_" + name + "_";
        dress(*model, "body", prefix + "body");
        dress(*model, "head", prefix + "head");
        dress(*model, "eyes", prefix + "eyes");
        dress(*model, "mouth", prefix + "mouth");
        dress(*model, "neck", prefix + "neck");
        dress(*model, "hair", prefix + "hair");
        dress(*model, "back_hair", prefix + "back_hair");
        dress(*model, "front_shoe", prefix + "foot");
        dress(*model, "back_shoe", prefix + "foot");
        dress(*model, "fronthand", prefix + "hand");
        dress(*model, "backhand", prefix + "hand2");
        dress(*model, "front_upperarm", prefix + "upperarm");
        dress(*model, "back_upperarm", prefix + "upperarm");
        dress(*model, "front_forearm", prefix + "forearm");
        dress(*model, "back_forearm", prefix + "forearm");
        dress(*model, "cross_backforearm", prefix + "forearm");
        dress(*model, "cross_back_hand", prefix + "hand2");
        const std::string weapon = "weapon_" + customers.getWeaponClipName(selected, gameObj->obf_51->selectedStyle);
        dress(*model, "weapon", weapon);
        dress(*model, "cross_back_weap", weapon);
        if (!isCaged) {
            const data::CustomerDataFile *c = customers.getCustomerData(selected);
            if (c && c->skillType == data::CustomerData::SKILL_GLIDE) {
                std::string glider = customers.getCustomerClipName(selected);
                if (c->gliderHasStyles && gameObj->obf_51->selectedStyle > 1) {
                    glider += std::to_string(static_cast<int>(gameObj->obf_51->selectedStyle));
                }
                dress(*model, "glider", "glider_" + glider);
            } else if (engine::MovieClip *g = model->findChild("glider")) {
                g->addChild(std::make_unique<engine::MovieClip>(symbols, emptyTimeline()), "");
            }
        }
        // scaleX/scaleY = modelScale and filters = [GlowFilter(0, 1, 1.3, 1.3, 4.68)]: not used by draw().
        models.push_back(std::move(model));
    }
}

// AS: CharacterBlitter.cleanupModel
void CharacterBlitter::cleanupModel()
{
    models.clear();
}

// AS: CharacterBlitter.destroy
void CharacterBlitter::destroy()
{
    cleanupModel();
    for (engine::Sheet *s : {&torsoSheet, &frontFootSheet, &backFootSheet}) {
        if (s->texture) {
            SDL_DestroyTexture(s->texture);
            s->texture = nullptr;
        }
    }
}

} // namespace papaGame::managers
