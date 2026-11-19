#include "game/papaGame/display/GameDisplay.h"

#include <algorithm>
#include <cmath>

#include "game/as3.h"
#include "game/obf_9G/obf_4R.h"

namespace papaGame::display {

// AS: GameDisplay.GameDisplay
GameDisplay::GameDisplay(obf_9G::obf_4R *gameObj_) : gameObj(gameObj_)
{
    centerXtile = std::ceil(screenTileWidth / 2);
    centerYtile = std::ceil(screenTileHeight / 2);
    currentLevel = gameObj->obf_AQ->currentLevel;
}

GameDisplay::~GameDisplay()
{
    if (tileLayer_) {
        SDL_DestroyTexture(tileLayer_);
    }
}

// AS: GameDisplay.getCamera
void GameDisplay::getCamera(GameCam *cam)
{
    camObj = cam;
}

// AS: GameDisplay.setupPreBlit
void GameDisplay::setupPreBlit()
{
    data::DataManager *manager = gameObj->obf_AQ.get();
    managers::BitmapManager *bitmaps = gameObj->bitmapManager.get();
    const data::ScreenData *screen = manager->currentScreenData;
    const double whichTileset = manager->currentTileset; // QUIRK: the original overwrites screen.whichTileset with this
    // blitLevelTiles(): the original pre-renders the room into screen.tileBMP, wrapped into strips of 90 tiles
    // (2880 px, the Flash bitmap limit). The port only keeps its size bookkeeping and draws the visible tiles
    // in reblitTiles, which gives the same pixels.
    tileSheet = bitmaps->getTilesheetBitmap(whichTileset);
    const auto &tiles = screen->tileArray;
    levelPxWidth = static_cast<double>(tiles[0].size()) * tileWidth;
    levelPxHeight = static_cast<double>(tiles.size()) * tileWidth;
    levelTileWidth = static_cast<double>(tiles[0].size());
    levelTileHeight = static_cast<double>(tiles.size());
    tileStamps_.clear(); // port: tileBMP is made anew (createBitmap)
    createBackground(whichTileset);
    currentXcoord = 0;
    currentYcoord = 0;
    showLightning = false;
}

// AS: GameDisplay.createBackground
double GameDisplay::createBackground(double whichTileset)
{
    managers::BitmapManager *bitmaps = gameObj->bitmapManager.get();
    if (bgSheet == nullptr) {
        bgSheet = bitmaps->getBackgroundBitmap(whichTileset);
    }
    if (mgSheet == nullptr) {
        mgSheet = bitmaps->getMiddlegroundBitmap(whichTileset);
    }
    return 0;
}

// AS: GameDisplay.defineScreenBounds
void GameDisplay::defineScreenBounds()
{
    const double tw = tileWidth;
    double left = camObj->x - std::floor(screenTileWidth / 2) * tileWidth;
    double top = camObj->y - std::floor(screenTileHeight / 2) * tileWidth;
    if (smooth_) {
        // Port: redrawScreen with smooth camera uses the interpolated, unsnapped position.
        left = drawCamX_ - std::floor(screenTileWidth / 2) * tileWidth;
        top = drawCamY_ - std::floor(screenTileHeight / 2) * tileWidth;
    } else {
        left = std::floor(left);
        top = std::floor(top);
    }
    currentXtileLeft = std::floor(left / tw);
    currentXtileRight = currentXtileLeft + screenTileWidth;
    currentYtileUp = std::floor(top / tw);
    currentYtileDown = currentYtileUp + screenTileHeight;
    currentXcoord = left;
    currentYcoord = top;
}

void GameDisplay::advanceFrame()
{
    prevTrainX_ = trainX;
    trainX += trainSpeed; // AS: from reblitSky
    blitTransition();
    if (deadIrisMC) {
        // AS: redrawScreen — when the iris clip reaches its last frame the level or the life is over.
        deadIrisMC->advance();
        if (deadIrisMC->currentFrame() == deadIrisMC->frameCount() - 1) {
            deadIrisMC.reset();
            auto &p = *gameObj->playerObj;
            if (gameObj->obf_AQ->currentLevel == 99 && p.isFinishingLevel) {
                SDL_Log("Should Show Outro"); // SKELETON: §_-14§ (the ending)
            } else if (p.isFinishingLevel) {
                SDL_Log("Should Beat Level:");
                gameObj->finishLevel();
            } else if (p.isDead && p.isRidingElevator) {
                SDL_Log("Should Reset Level (Died in Elevator)");
                gameObj->obf_99();
            } else {
                SDL_Log("Should Die");
                gameObj->obf_2K();
            }
        }
    }
}

// AS: GameDisplay.startMainIrisOut
void GameDisplay::startMainIrisOut(double x, double y)
{
    SDL_Log("Start Main Irising Out");
    if (deadIrisMC) {
        return;
    }
    const engine::SymbolLibrary &symbols = *gameObj->symbols;
    for (const auto &[name, id] : symbols.classes()) {
        if (name == "deadIrisOut") {
            if (const engine::Timeline *tl = symbols.timeline(id)) {
                deadIrisMC = std::make_unique<engine::MovieClip>(symbols, *tl);
            }
        }
    }
    if (!deadIrisMC) {
        return;
    }
    deadIrisX = gameObj->playerObj->x - currentXcoord;
    deadIrisY = gameObj->playerObj->y - currentYcoord;
    if (x != -1) {
        deadIrisX = x - currentXcoord;
        deadIrisY = y - currentYcoord;
    }
    deadIrisMC->gotoAndPlay(1);
}

// AS: GameDisplay.redrawScreen
void GameDisplay::redrawScreen(float alpha, bool smooth)
{
    const double a = static_cast<double>(alpha);
    drawTrainX_ = prevTrainX_ + (trainX - prevTrainX_) * a;
    drawCamX_ = camObj->prevX + (camObj->x - camObj->prevX) * a;
    drawCamY_ = camObj->prevY + (camObj->y - camObj->prevY) * a;
    if (!smooth) {
        // Original: integer stage pixels of the latest logic frame.
        drawCamX_ = camObj->x;
        drawCamY_ = camObj->y;
        drawTrainX_ = trainX;
    }
    smooth_ = smooth;
    drawAlpha_ = smooth ? a : 1.0;
    defineScreenBounds();
    reblitSky();
    reblitBackdrop();
    reblitTiles();
    reblitItems(true, false);
    reblitEffects(false);
    reblitObjects(false);
    reblitItems(false, false);
    reblitEnemies(false);
    reblitBullets(false);
    const models::characters::CustomerChar *player = gameObj->playerObj.get();
    if (player && player->blitter &&
        (!player->onMovingTile || player->isRidingElevator ||
         (player->whichMovingTile != nullptr && player->whichMovingTile->foreground == false))) {
        reblitMultispritePlayer();
    }
    reblitObjects(true);
    if (player && player->blitter && player->onMovingTile && player->whichMovingTile != nullptr &&
        player->whichMovingTile->foreground == true && !player->isRidingElevator) {
        reblitMultispritePlayer();
    }
    reblitEnemies(true);
    reblitEffects(true);
    reblitItems(false, true);
    reblitBullets(true);
    drawTransition();
    // AS: blitEffectsOverlay — the white flash when the player enters the end portal.
    // SKELETON: the rain and lightning of level 11.
    if (flashPortal) {
        SDL_SetRenderDrawColor(gameObj->renderer, 255, 255, 255, 255);
        const SDL_FRect all{0, 0, static_cast<float>(screenPxWidth), static_cast<float>(screenPxHeight)};
        SDL_RenderFillRect(gameObj->renderer, &all);
        flashPortal = false;
    }
    // Port: the HUD and the iris sit above the game display in the original's display list.
    // SKELETON: masks are not drawn yet, so the iris shows its unmasked shapes.
    if (deadIrisMC) {
        const engine::RenderContext ctx{gameObj->renderer, gameObj->symbols, gameObj->textures, gameObj->text};
        deadIrisMC->render(ctx, engine::Affine::translate(static_cast<float>(deadIrisX), static_cast<float>(deadIrisY)),
                           engine::ColorTransform{});
    }
    if (gameObj->obf_6Z) {
        const engine::RenderContext ctx{gameObj->renderer, gameObj->symbols, gameObj->textures, gameObj->text};
        gameObj->obf_6Z->render(ctx);
    }
    smooth_ = false;
}

// AS: GameDisplay.startTransition
void GameDisplay::startTransition(const std::string &type)
{
    const managers::BitmapManager &bitmaps = *gameObj->bitmapManager;
    if (type == "in") {
        isTransitioningIn = true;
        const engine::Sheet *sheet = bitmaps.sheets->get("transition_in_effect");
        transitionX = 0 - (sheet ? sheet->width : 0);
        // SKELETON: the HUD is shown here.
    } else if (type == "out") {
        isTransitionOut = true;
        const engine::Sheet *sheet = bitmaps.sheets->get("transition_out_effect");
        transitionX = 0 - (sheet ? sheet->width : 0);
    }
    prevTransitionX_ = transitionX;
}

// AS: GameDisplay.blitTransition (the movement; drawTransition draws it)
void GameDisplay::blitTransition()
{
    prevTransitionX_ = transitionX;
    if (isTransitioningIn || isTransitionOut) {
        transitionX += transitionSpeed;
        if (transitionX > screenPxWidth) {
            finishTransition();
        }
    }
}

// AS: GameDisplay.finishTransition
void GameDisplay::finishTransition()
{
    events::GameControls &controls = *gameObj->obf_8o;
    if (isTransitioningIn) {
        isTransitioningIn = false;
    } else if (isTransitionOut) {
        isTransitionOut = false;
        if (controls.travelingThroughDoor) {
            gameObj->obf_4X(controls.doorRoomID, controls.doorDoorID, controls.doorXoffset, controls.doorYoffset,
                            controls.doorDirX, controls.doorDirY);
            controls.travelingThroughDoor = false;
        }
        // SKELETON: otherwise game over (§_-2K§) or finishLevel.
    }
}

// The drawing half of GameDisplay.blitTransition: the wipe sheet at transitionX, black on the covered side.
void GameDisplay::drawTransition()
{
    if (!isTransitioningIn && !isTransitionOut) {
        return;
    }
    const managers::BitmapManager &bitmaps = *gameObj->bitmapManager;
    SDL_Renderer *renderer = gameObj->renderer;
    const double x = prevTransitionX_ + (transitionX - prevTransitionX_) * drawAlpha_;
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xFF);
    if (isTransitioningIn) {
        const engine::Sheet *sheet = bitmaps.sheets->get("transition_in_effect");
        const double w = sheet ? sheet->width : 0;
        if (sheet) {
            blit(*sheet, 0, 0, w, sheet->height, x, 0);
        }
        if (x + w < screenPxWidth) {
            const SDL_FRect rect{static_cast<float>(x + w), 0, static_cast<float>(screenPxWidth - (x + w)),
                                 static_cast<float>(screenPxHeight)};
            SDL_RenderFillRect(renderer, &rect);
        }
    } else {
        const engine::Sheet *sheet = bitmaps.sheets->get("transition_out_effect");
        if (sheet) {
            blit(*sheet, 0, 0, sheet->width, sheet->height, x, 0);
        }
        if (x < screenPxWidth) {
            const SDL_FRect rect{0, 0, static_cast<float>(x), static_cast<float>(screenPxHeight)};
            SDL_RenderFillRect(renderer, &rect);
        }
    }
}

void GameDisplay::blit(const engine::Sheet &sheet, double sx, double sy, double sw, double sh, double dx,
                       double dy) const
{
    gameObj->bitmapManager->sheets->blit(sheet, static_cast<float>(sx), static_cast<float>(sy),
                                         static_cast<float>(sw), static_cast<float>(sh), static_cast<float>(dx),
                                         static_cast<float>(dy));
}

void GameDisplay::stampOnTiles(const engine::Sheet &sheet, double sx, double sy, double sw, double sh, double dx,
                               double dy)
{
    tileStamps_.push_back(TileStamp{&sheet, sx, sy, sw, sh, dx, dy});
}

// AS: GameDisplay.reblitSky
void GameDisplay::reblitSky()
{
    const data::DataManager *manager = gameObj->obf_AQ.get();
    SDL_Renderer *renderer = gameObj->renderer;
    // screenBMP.fillRect(screenBMP.rect, 0xFFFFCC00)
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xCC, 0x00, 0xFF);
    const SDL_FRect rect{0, 0, static_cast<float>(screenPxWidth), static_cast<float>(screenPxHeight)};
    SDL_RenderFillRect(renderer, &rect);
    if (bgSheet == nullptr) {
        return;
    }
    const engine::Sheet &bg = *bgSheet;
    double x = currentXcoord;
    if (manager->isOnTrain() || manager->isAnimatedBackground()) {
        x += drawTrainX_;
    }
    const double w = bg.width;
    const double h = bg.height;
    double dx = smooth_ ? x / 8 : as3::round(x / 8);
    const double dy = 0;
    if (dx > w) {
        dx = std::fmod(dx, w);
    }
    dx = w - dx;
    const double rightW = w - dx;
    const double drawH = h - dy;
    const double leftW = w - rightW;
    // Port: copyPixels without mergeAlpha replaces the pixels; the sky art is opaque, so blending is the same.
    blit(bg, 0, 0, rightW, drawH, dx, dy);
    if (dx > 0) {
        blit(bg, rightW, 0, leftW, drawH, 0, dy);
    }
    if (showLightning && std::fmod(lightningTimer, 2) == 1) {
        SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        SDL_RenderFillRect(renderer, &rect);
    }
}

// AS: GameDisplay.reblitBackdrop
void GameDisplay::reblitBackdrop()
{
    if (mgSheet == nullptr) {
        return;
    }
    const engine::Sheet &mg = *mgSheet;
    const data::DataManager *manager = gameObj->obf_AQ.get();
    const double mgW = mg.width;
    const double mgH = mg.height;
    double x = currentXcoord;
    const double y = currentYcoord;
    if (manager->isOnTrain() || manager->isAnimatedBackground()) {
        x += drawTrainX_;
    }
    const auto snap = [this](double v) { return smooth_ ? v : as3::round(v); };
    if (bgScrollType == "static") {
        blit(mg, 0, 0, mgW, mgH, 0, 0);
        return;
    }
    if (bgScrollType != "attached" && bgScrollType != "scroll") {
        return;
    }
    double dx = 0;
    double dy = 0;
    if (bgScrollType == "attached") {
        dx = mgW - std::fmod(x, mgW);
        dy = mgH - std::fmod(y, mgH);
    } else {
        dx = snap(x / bgScrollSpeedX);
        dy = snap(y / bgScrollSpeedY);
        if (dx > mgW) {
            dx = std::fmod(dx, mgW);
        }
        if (dy > mgH) {
            dy = std::fmod(dy, mgH);
        }
        dx = mgW - dx;
        dy = mgH - dy;
        if (bgScrollSpeedX == 0) {
            dx = mgW - std::fmod(x, mgW);
        }
        if (bgScrollSpeedY == 0) {
            dy = mgH - std::fmod(y, mgH);
        }
    }
    // The backdrop's vertical position follows how far down the level the camera is.
    double range = levelPxHeight - screenPxHeight;
    if (range < screenPxHeight) {
        range = screenPxHeight;
    }
    const double t = std::min(1.0, y / range);
    const double travel = levelPxHeight - mgH;
    dy = t * travel - y;
    if (!smooth_) {
        dy = std::floor(dy);
    }
    const double rightW = mgW - dx;
    const double bottomH = mgH - dy;
    const double leftW = mgW - rightW;
    const double topH = mgH - bottomH;
    blit(mg, 0, 0, rightW, bottomH, dx, dy);
    if (dx > 0) {
        blit(mg, rightW, 0, leftW, bottomH, 0, dy);
    }
    if (dy > 0) {
        blit(mg, 0, bottomH, rightW, topH, dx, 0);
        if (dx > 0) {
            blit(mg, rightW, bottomH, leftW, topH, 0, 0);
        }
    }
}

// AS: GameDisplay.reblitTiles
void GameDisplay::reblitTiles()
{
    // The original copies the visible 700x416 window out of the pre-rendered tileBMP (see setupPreBlit).
    // Drawing the cells under that window, each layer in order with mergeAlpha, is the same picture.
    if (tileSheet == nullptr) {
        return;
    }
    const auto &tiles = gameObj->obf_AQ->currentScreenData->tileArray;
    SDL_Renderer *renderer = gameObj->renderer;
    const float s = tileSheet->scale;
    // Port: tiles are drawn at whole stage pixels into tileLayer_ with nearest sampling (no bleeding from
    // neighbouring cells of the tilesheet), then the layer is placed at the sub-pixel camera offset.
    if (tileLayer_ == nullptr || tileLayerScale_ != s) {
        if (tileLayer_) {
            SDL_DestroyTexture(tileLayer_);
        }
        tileLayer_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                       static_cast<int>((screenPxWidth + 2) * s),
                                       static_cast<int>((screenPxHeight + 2) * s));
        if (tileLayer_ == nullptr) {
            return;
        }
        SDL_SetTextureBlendMode(tileLayer_, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
        SDL_SetTextureScaleMode(tileLayer_, SDL_SCALEMODE_LINEAR);
        tileLayerScale_ = s;
    }
    const double x0 = std::floor(currentXcoord), y0 = std::floor(currentYcoord);
    SDL_Texture *screen = SDL_GetRenderTarget(renderer);
    SDL_SetRenderTarget(renderer, tileLayer_);
    SDL_SetRenderScale(renderer, s, s);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetTextureScaleMode(tileSheet->texture, SDL_SCALEMODE_NEAREST);
    const auto rowFirst = static_cast<long>(std::floor(y0 / tileWidth));
    const auto rowLast = static_cast<long>(std::floor((y0 + screenPxHeight - 1) / tileWidth));
    const auto colFirst = static_cast<long>(std::floor(x0 / tileWidth));
    const auto colLast = static_cast<long>(std::floor((x0 + screenPxWidth - 1) / tileWidth));
    for (long r = std::max(rowFirst, 0L); r <= rowLast && r < static_cast<long>(tiles.size()); ++r) {
        const auto &row = tiles[static_cast<size_t>(r)];
        for (long c = std::max(colFirst, 0L); c <= colLast && c < static_cast<long>(row.size()); ++c) {
            for (const double v : row[static_cast<size_t>(c)]) {
                // AS: blitLevelTiles — tile v sits at cell (v mod 62, floor(v / 62)) of the tilesheet.
                const double sx = (v - std::floor(v / tileSheetWidth) * tileSheetWidth) * tileWidth;
                const double sy = std::floor(v / tileSheetWidth) * tileWidth;
                blit(*tileSheet, sx, sy, tileWidth, tileWidth, static_cast<double>(c) * tileWidth - x0,
                     static_cast<double>(r) * tileWidth - y0);
            }
        }
    }
    SDL_SetTextureScaleMode(tileSheet->texture, SDL_SCALEMODE_LINEAR);
    for (const TileStamp &st : tileStamps_) {
        blit(*st.sheet, st.sx, st.sy, st.sw, st.sh, st.dx - x0, st.dy - y0);
    }
    SDL_SetRenderTarget(renderer, screen);
    const SDL_FRect src{0, 0, static_cast<float>(screenPxWidth + 1) * s, static_cast<float>(screenPxHeight + 1) * s};
    const SDL_FRect dst{static_cast<float>(x0 - currentXcoord), static_cast<float>(y0 - currentYcoord),
                        static_cast<float>(screenPxWidth + 1), static_cast<float>(screenPxHeight + 1)};
    SDL_RenderTexture(renderer, tileLayer_, &src, &dst);
}

// AS: GameDisplay.reblitMultispritePlayer
void GameDisplay::reblitMultispritePlayer()
{
    const double xcoord = currentXcoord, ycoord = currentYcoord;
    const models::characters::CustomerChar &p = *gameObj->playerObj;
    const managers::CharacterBlitter &b = *p.blitter;
    if (p.isStunned) {
        if (++spriteFlickerCounter > 1) {
            spriteFlickerCounter = 0;
        }
    } else {
        spriteFlickerCounter = 0;
    }
    // Port: with smooth rendering the player is drawn between its last two logic positions.
    const double px = p.prevX + (p.x - p.prevX) * drawAlpha_;
    const double py = p.prevY + (p.y - p.prevY) * drawAlpha_;
    const double facing = p.facingDir;
    double x = px - (b.spriteTargetX - b.spriteCenterX);
    const double y = py - (b.spriteTargetY - b.spriteCenterY);
    x -= facing == -1 ? b.blitOffsetX : b.blitFlippedOffsetX;
    double footX = x - xcoord, footY = y - ycoord - b.footSpriteHeight / 2;
    if (!smooth_) {
        footX = std::floor(footX);
        footY = std::floor(footY);
    }
    double torsoX = footX, torsoY = footY;
    if (facing == 1) {
        torsoX -= b.torsoOffsetFlippedX;
        torsoY += b.torsoOffsetY;
        torsoX += p.getTorsoOffsetX();
        torsoY += p.getTorsoOffsetY();
    } else if (facing == -1) {
        torsoX += b.torsoOffsetX;
        torsoY += b.torsoOffsetY;
        torsoX -= p.getTorsoOffsetX();
        torsoY += p.getTorsoOffsetY();
    }
    // Cell `frame` of a sheet with `perRow` cells of w x h per row, plus the half offset for the facing.
    auto cell = [&](const engine::Sheet &sheet, double frame, double w, double h, double perRow, double flipOffset,
                    double dx, double dy) {
        if (!sheet.texture) {
            return;
        }
        const double sx = (frame - std::floor(frame / perRow) * perRow) * w;
        const double sy = std::floor(frame / perRow) * h;
        blit(sheet, sx, sy + flipOffset, w, h, dx, dy);
    };
    const double footFlip = facing == -1 ? b.footFlipBlitOffset : 0;
    cell(b.backFootSheet, p.footAnimationFrame, b.footSpriteWidth, b.footSpriteHeight, b.footSpritesPerRow, footFlip,
         footX, footY);
    cell(b.torsoSheet, p.animationFrame, b.spriteWidth, b.spriteHeight, b.spritesPerRow,
         facing == -1 ? b.flipBlitOffset : 0, torsoX, torsoY);
    cell(b.frontFootSheet, p.footAnimationFrame, b.footSpriteWidth, b.footSpriteHeight, b.footSpritesPerRow, footFlip,
         footX, footY);
    // SKELETON: blitCollisionArea (debug bounds) is not drawn.
}

// AS: GameDisplay.reblitBullets — each bullet's frame, in the row block for its direction (grabFlipOffset).
void GameDisplay::reblitBullets(bool foreground)
{
    managers::BulletManager &manager = *gameObj->obf_4l;
    for (models::Bullet *b : manager.bullets) {
        if (!b || !checkOnScreen(*b) || b->foreground != foreground) {
            continue;
        }
        const double frame = b->animationFrame;
        const double w = b->spritewidth, h = b->spriteheight;
        const double sx = (frame - std::floor(frame / b->sheetWidth) * b->sheetWidth) * w;
        const double sy = std::floor(frame / b->sheetWidth) * h + b->grabFlipOffset();
        const double bx = b->prevX + (b->x - b->prevX) * drawAlpha_;
        const double by = b->prevY + (b->y - b->prevY) * drawAlpha_;
        double dx = bx - (b->spriteTargetX - b->spriteCenterX) - currentXcoord - w / 2;
        double dy = by - (b->spriteTargetY - b->spriteCenterY) - currentYcoord - h / 2;
        if (!smooth_) {
            dx = std::floor(dx);
            dy = std::floor(dy);
        }
        if (const engine::Sheet *sheet = b->getSheet()) {
            blit(*sheet, sx, sy, w, h, dx, dy);
        }
    }
}

} // namespace papaGame::display

namespace papaGame::display {

// AS: GameDisplay.checkOnScreen — within checkOnScreenOffset tiles of the visible area.
bool GameDisplay::checkOnScreen(const models::Character &c) const
{
    const double offset = checkOnScreenOffset;
    const double w = c.widthmultiplier - 0.5;
    const double h = c.heightmultiplier - 0.5;
    if (c.xtile + w < currentXtileLeft - offset) {
        return false;
    }
    if (c.xtile > currentXtileRight + offset) {
        return false;
    }
    if (c.ytile < currentYtileUp - offset) {
        return false;
    }
    if (c.ytile - h > currentYtileDown + offset) {
        return false;
    }
    return true;
}

// AS: GameDisplay.reblitEnemies (SKELETON: reblitMultispriteEnemy; blitCollisionArea is a debug view)
void GameDisplay::reblitEnemies(bool foreground)
{
    managers::EnemyManager &manager = *gameObj->obf_E;
    for (models::Enemy *e : manager.enemies) {
        if (!e || e->foreground != foreground || !e->checkOnScreen() || e->isMultiSprite) {
            continue;
        }
        const double frame = e->animationFrame;
        const double w = e->spritewidth, h = e->spriteheight;
        const double perRow = e->sheetWidth;
        const double flip = e->facingDir == -1 ? e->flipBlitOffset : 0;
        const double sx = (frame - std::floor(frame / perRow) * perRow) * w;
        const double sy = std::floor(frame / perRow) * h + flip;
        // Port: with smooth rendering the enemy is drawn between its last two logic positions.
        const double ex = e->prevX + (e->x - e->prevX) * drawAlpha_;
        const double ey = e->prevY + (e->y - e->prevY) * drawAlpha_;
        double dx = ex - (e->spriteTargetX - e->spriteCenterX) - currentXcoord - w / 2;
        double dy = ey - (e->spriteTargetY - e->spriteCenterY) - currentYcoord - h / 2;
        if (!smooth_) {
            dx = std::floor(dx);
            dy = std::floor(dy);
        }
        e->checkBeforeBlitting();
        const auto index = static_cast<size_t>(e->sheetIndex);
        if (e->sheetIndex >= 0 && index < manager.objectSheets.size() && manager.objectSheets[index]) {
            blit(*manager.objectSheets[index], sx, sy, w, h, dx, dy);
        }
    }
}

// AS: GameDisplay.reblitObjects (SKELETON: blitTilingObject, blitCollisionArea)
void GameDisplay::reblitObjects(bool foreground)
{
    managers::ObjectManager &manager = *gameObj->obf_7u;
    // The object the player stands on is drawn last, over the others.
    const size_t count = manager.objects.size();
    for (size_t i = 0; i <= count; ++i) {
        models::GameObject *o = nullptr;
        if (i < count) {
            o = manager.objects[i];
        } else if (gameObj->playerObj->onMovingTile) {
            o = gameObj->playerObj->whichMovingTile;
        }
        if (!o || o->foreground != foreground || !o->checkOnScreen()) {
            continue;
        }
        if (gameObj->playerObj->whichMovingTile == o && i != count) {
            continue;
        }
        if (o->isLadder) {
            blitLadder(*o);
            continue;
        }
        const double frame = o->animationFrame;
        const double w = o->spritewidth, h = o->spriteheight;
        const double perRow = o->sheetWidth;
        const double flip = o->facingDir == -1 ? o->flipBlitOffset : 0;
        const double sx = (frame - std::floor(frame / perRow) * perRow) * w;
        const double sy = std::floor(frame / perRow) * h + flip;
        const double ox = o->prevX + (o->x - o->prevX) * drawAlpha_;
        const double oy = o->prevY + (o->y - o->prevY) * drawAlpha_;
        double dx = ox - (o->spriteTargetX - o->spriteCenterX) - currentXcoord - w / 2;
        double dy = oy - (o->spriteTargetY - o->spriteCenterY) - currentYcoord - h / 2;
        if (!smooth_) {
            dx = std::floor(dx);
            dy = std::floor(dy);
        }
        o->checkAdditionalBlitting(foreground, true);
        const auto index = static_cast<size_t>(o->sheetIndex);
        if (o->sheetIndex >= 0 && index < manager.objectSheets.size() && manager.objectSheets[index]) {
            blit(*manager.objectSheets[index], sx, sy, w, h, dx, dy);
        }
        o->checkAdditionalBlitting(foreground);
    }
}

// AS: GameDisplay.blitLadder — the rungs from the bottom tile up, then the bottom and top caps.
void GameDisplay::blitLadder(const models::GameObject &o)
{
    const managers::ObjectManager &manager = *gameObj->obf_7u;
    const auto index = static_cast<size_t>(o.sheetIndex);
    if (o.sheetIndex < 0 || index >= manager.objectSheets.size() || !manager.objectSheets[index]) {
        return;
    }
    const engine::Sheet &sheet = *manager.objectSheets[index];
    if (!o.isActivated) {
        return; // "Ladder not activated?!"
    }
    const double offsetX = o.spriteTargetX - o.spriteCenterX;
    const double offsetY = o.spriteTargetY - o.spriteCenterY;
    const double h = o.tilingHeight;
    if (h > 1) {
        const double dx = o.xtile * tileWidth - currentXcoord - offsetX;
        for (int i = 0; i < h; ++i) {
            blit(sheet, o.tilingFrame * o.spritewidth, 0, o.spritewidth, o.spriteheight, dx,
                 (o.ytile - i) * tileWidth - currentYcoord - offsetY);
        }
        blit(sheet, o.bottomCapFrame * o.spritewidth, 0, o.spritewidth, o.spriteheight, dx,
             (o.ytile + 1) * tileWidth - currentYcoord - offsetY);
        blit(sheet, o.topCapFrame * o.spritewidth, 0, o.spritewidth, o.spriteheight, dx,
             (o.ytile - h) * tileWidth - currentYcoord - offsetY);
    }
}

// AS: GameDisplay.reblitItems
void GameDisplay::reblitItems(bool background, bool foreground)
{
    managers::ItemManager &manager = *gameObj->obf_3Z;
    for (models::GameItem *it : manager.items) {
        if (!it || it->background != background || it->foreground != foreground || !it->checkOnScreen()) {
            continue;
        }
        const double frame = it->animationFrame;
        const double w = it->spritewidth, h = it->spriteheight;
        const double sx = (frame - std::floor(frame / it->sheetWidth) * it->sheetWidth) * w;
        const double sy = std::floor(frame / it->sheetWidth) * h;
        const double ix = it->prevX + (it->x - it->prevX) * drawAlpha_;
        const double iy = it->prevY + (it->y - it->prevY) * drawAlpha_;
        double dx = ix - (it->spriteTargetX - it->spriteCenterX) - currentXcoord - w / 2;
        double dy = iy - (it->spriteTargetY - it->spriteCenterY) - currentYcoord - h / 2;
        if (!smooth_) {
            dx = std::floor(dx);
            dy = std::floor(dy);
        }
        const auto index = static_cast<size_t>(it->sheetIndex);
        if (it->sheetIndex >= 0 && index < manager.objectSheets.size() && manager.objectSheets[index]) {
            blit(*manager.objectSheets[index], sx, sy, w, h, dx, dy);
        }
    }
}

// AS: GameDisplay.reblitEffects (SKELETON: NumberEffect / PopocaEffect draw from their own sheet; PL3 only)
void GameDisplay::reblitEffects(bool foreground)
{
    managers::EffectManager &manager = *gameObj->obf_V;
    for (models::Effect *e : manager.effects) {
        if (!e || e->foreground != foreground || !e->checkOnScreen()) {
            continue;
        }
        const double frame = e->animationFrame;
        const double w = e->spritewidth, h = e->spriteheight;
        const double sx = (frame - std::floor(frame / e->sheetWidth) * e->sheetWidth) * w;
        const double sy = std::floor(frame / e->sheetWidth) * h + e->flipBlitOffset;
        const double ex = e->prevX + (e->x - e->prevX) * drawAlpha_;
        const double ey = e->prevY + (e->y - e->prevY) * drawAlpha_;
        double dx = ex - (e->spriteTargetX - e->spriteCenterX) - currentXcoord - w / 2;
        double dy = ey - (e->spriteTargetY - e->spriteCenterY) - currentYcoord - h / 2;
        if (!smooth_) {
            dx = std::floor(dx);
            dy = std::floor(dy);
        }
        if (const engine::Sheet *sheet = e->getSheet()) {
            blit(*sheet, sx, sy, w, h, dx, dy);
        }
    }
}

} // namespace papaGame::display
