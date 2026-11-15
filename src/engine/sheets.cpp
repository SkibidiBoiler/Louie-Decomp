#include "engine/sheets.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace engine {

namespace {

// BitmapManager.sheetParts / sheetHeights / fullSheetWidth / fullSheetHeight.
constexpr const char *kTilesheetParts[] = {"solid", "soft", "bridge", "blocks", "details"};
constexpr float kTilesheetHeights[] = {384.0f, 384.0f, 128.0f, 96.0f, 320.0f};
constexpr float kTilesheetWidth = 1984.0f;
constexpr float kTilesheetHeight = 1312.0f;

SDL_Texture *loadTexture(SDL_Renderer *renderer, const std::string &path)
{
    SDL_Surface *surface = SDL_LoadPNG(path.c_str());
    if (!surface) {
        return nullptr;
    }
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_DestroySurface(surface);
    if (texture) {
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }
    return texture;
}

} // namespace

bool SheetLibrary::init(SDL_Renderer *renderer, const std::string &gameDir, int scale)
{
    renderer_ = renderer;
    scale_ = scale;
    dir_ = gameDir + "/sheets/";
    size_t size = 0;
    char *text = static_cast<char *>(SDL_LoadFile((dir_ + "index.txt").c_str(), &size));
    if (!text) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "No sheets index: %s", SDL_GetError());
        return false;
    }
    std::string all(text, size);
    SDL_free(text);
    size_t pos = 0;
    while (pos < all.size()) {
        size_t end = all.find('\n', pos);
        if (end == std::string::npos) {
            end = all.size();
        }
        const std::string line = all.substr(pos, end - pos);
        pos = end + 1;
        char name[256] = {};
        char type[16] = {};
        float ox = 0.0f, oy = 0.0f;
        if (std::sscanf(line.c_str(), "%255s %f %f %15s", name, &ox, &oy, type) == 4) {
            index_[name] = Entry{ox, oy, std::string(type) == "bitmap"};
        }
    }
    SDL_Log("Sheets: %zu in index, vector scale %dx", index_.size(), scale_);
    return true;
}

void SheetLibrary::clear()
{
    for (auto &entry : loaded_) {
        if (entry.second.texture) {
            SDL_DestroyTexture(entry.second.texture);
        }
    }
    loaded_.clear();
}

const Sheet *SheetLibrary::get(const std::string &name)
{
    auto hit = loaded_.find(name);
    if (hit != loaded_.end()) {
        return hit->second.texture ? &hit->second : nullptr;
    }
    Sheet sheet;
    auto it = index_.find(name);
    if (it != index_.end()) {
        const Entry &e = it->second;
        if (e.bitmap) {
            for (const char *ext : {".png", ".jpg"}) {
                if ((sheet.texture = loadTexture(renderer_, dir_ + "1x/" + name + ext)) != nullptr) {
                    break;
                }
            }
            sheet.scale = 1.0f;
        } else {
            sheet.texture = loadTexture(renderer_, dir_ + std::to_string(scale_) + "x/" + name + ".png");
            sheet.scale = static_cast<float>(scale_);
        }
        sheet.originX = e.originX;
        sheet.originY = e.originY;
    }
    if (!sheet.texture) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Sheet '%s' not available", name.c_str());
    }
    auto &slot = loaded_[name];
    slot = sheet;
    return slot.texture ? &slot : nullptr;
}

const Sheet *SheetLibrary::composeTilesheet(const std::string &name)
{
    auto hit = loaded_.find(name);
    if (hit != loaded_.end()) {
        return hit->second.texture ? &hit->second : nullptr;
    }
    const float s = static_cast<float>(scale_);
    Sheet out;
    out.scale = s;
    out.premultiplied = true; // straight-alpha parts blended onto a transparent target
    out.texture = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
                                    static_cast<int>(kTilesheetWidth * s), static_cast<int>(kTilesheetHeight * s));
    if (!out.texture) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Tilesheet target: %s", SDL_GetError());
        loaded_[name] = out;
        return nullptr;
    }
    SDL_SetTextureScaleMode(out.texture, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(out.texture, SDL_BLENDMODE_BLEND_PREMULTIPLIED);

    SDL_Texture *previous = SDL_GetRenderTarget(renderer_);
    SDL_SetRenderTarget(renderer_, out.texture);
    SDL_SetRenderDrawColor(renderer_, 0, 0, 0, 0);
    SDL_RenderClear(renderer_);
    float top = 0.0f;
    for (size_t i = 0; i < std::size(kTilesheetParts); ++i) {
        const std::string partName = name + "_" + kTilesheetParts[i];
        if (const Sheet *part = get(partName)) {
            // draw(part, Matrix(ty = top), clipRect = Rectangle(0, top, width, height))
            const SDL_Rect clip{0, static_cast<int>(top * s), static_cast<int>(kTilesheetWidth * s),
                                static_cast<int>(kTilesheetHeights[i] * s)};
            SDL_SetRenderClipRect(renderer_, &clip);
            float tw = 0.0f, th = 0.0f;
            SDL_GetTextureSize(part->texture, &tw, &th);
            const SDL_FRect dst{part->originX * s, (top + part->originY) * s, tw * s / part->scale,
                                th * s / part->scale};
            SDL_RenderTexture(renderer_, part->texture, nullptr, &dst);
        }
        top += kTilesheetHeights[i];
    }
    SDL_SetRenderClipRect(renderer_, nullptr);
    SDL_SetRenderTarget(renderer_, previous);

    // The parts are only needed to build the composite.
    for (const char *part : kTilesheetParts) {
        auto it = loaded_.find(name + "_" + part);
        if (it != loaded_.end()) {
            if (it->second.texture) {
                SDL_DestroyTexture(it->second.texture);
            }
            loaded_.erase(it);
        }
    }
    auto &slot = loaded_[name];
    slot = out;
    return &slot;
}

void SheetLibrary::blit(const Sheet &sheet, float sx, float sy, float sw, float sh, float dx, float dy) const
{
    // Source rect in texture pixels; sheet unit (u, v) lives at ((u - origin) * scale) in the texture.
    float tw = 0.0f, th = 0.0f;
    SDL_GetTextureSize(sheet.texture, &tw, &th);
    float x0 = (sx - sheet.originX) * sheet.scale, y0 = (sy - sheet.originY) * sheet.scale;
    float x1 = x0 + sw * sheet.scale, y1 = y0 + sh * sheet.scale;
    // Clip to the texture like copyPixels clips to the source bitmap; shift the destination to match.
    const float cx0 = std::max(x0, 0.0f), cy0 = std::max(y0, 0.0f);
    const float cx1 = std::min(x1, tw), cy1 = std::min(y1, th);
    if (cx1 <= cx0 || cy1 <= cy0) {
        return;
    }
    const SDL_FRect src{cx0, cy0, cx1 - cx0, cy1 - cy0};
    const SDL_FRect dst{dx + (cx0 - x0) / sheet.scale, dy + (cy0 - y0) / sheet.scale, src.w / sheet.scale,
                        src.h / sheet.scale};
    SDL_RenderTexture(renderer_, sheet.texture, &src, &dst);
}

} // namespace engine
