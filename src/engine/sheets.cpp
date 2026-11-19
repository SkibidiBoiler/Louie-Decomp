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

// Port: the vector parts are baked with their own bounds as the pixel origin (e.g. -24.67), so the 32-unit tile
// grid falls between pixels and each tile edge is a half-covered, see-through pixel; side by side the tiles show
// a seam. Flash drew the clip at a whole-pixel origin, where the edges are solid. Each edge pixel of a cell that
// is more transparent than the pixel just inside takes that pixel instead.
void repairPixelPair(Uint32 *edge, const Uint32 *inside)
{
    if ((*inside >> 24) > (*edge >> 24)) {
        *edge = *inside;
    }
}

} // namespace

// Port: see repairPixelPair. Runs once on the composed tilesheet (the render target is set to it).
void SheetLibrary::repairCellEdges(Sheet &sheet, float cell)
{
    SDL_Surface *read = SDL_RenderReadPixels(renderer_, nullptr);
    if (!read) {
        return;
    }
    SDL_Surface *px = SDL_ConvertSurface(read, SDL_PIXELFORMAT_ARGB8888);
    SDL_DestroySurface(read);
    if (!px) {
        return;
    }
    const int w = px->w, h = px->h, c = static_cast<int>(cell);
    auto at = [px](int x, int y) {
        return reinterpret_cast<Uint32 *>(static_cast<Uint8 *>(px->pixels) + y * px->pitch) + x;
    };
    // Only the outermost pixel: repairing two pixels deep also thickens art that merely crosses a cell edge.
    if (c >= 4) {
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x + c <= w; x += c) {
                repairPixelPair(at(x, y), at(x + 1, y));
                repairPixelPair(at(x + c - 1, y), at(x + c - 2, y));
            }
        }
        for (int y = 0; y + c <= h; y += c) {
            for (int x = 0; x < w; ++x) {
                repairPixelPair(at(x, y), at(x, y + 1));
                repairPixelPair(at(x, y + c - 1), at(x, y + c - 2));
            }
        }
    }
    SDL_Texture *fixed = SDL_CreateTextureFromSurface(renderer_, px);
    SDL_DestroySurface(px);
    if (!fixed) {
        return;
    }
    SDL_SetTextureScaleMode(fixed, SDL_SCALEMODE_LINEAR);
    SDL_SetTextureBlendMode(fixed, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
    SDL_SetRenderTarget(renderer_, nullptr); // the caller restores its own target next
    SDL_DestroyTexture(sheet.texture);
    sheet.texture = fixed;
}

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
        float ox = 0.0f, oy = 0.0f, w = 0.0f, h = 0.0f;
        // <name> <originX> <originY> vector|bitmap [<width> <height>]
        if (std::sscanf(line.c_str(), "%255s %f %f %15s %f %f", name, &ox, &oy, type, &w, &h) >= 4) {
            index_[name] = Entry{ox, oy, w, h, std::string(type) == "bitmap"};
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
        sheet.width = e.width;
        sheet.height = e.height;
        if (sheet.texture && (sheet.width <= 0.0f || sheet.height <= 0.0f)) {
            float tw = 0.0f, th = 0.0f;
            SDL_GetTextureSize(sheet.texture, &tw, &th);
            sheet.width = tw / sheet.scale;
            sheet.height = th / sheet.scale;
        }
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
    out.width = kTilesheetWidth;
    out.height = kTilesheetHeight;
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
    repairCellEdges(out, 32.0f * s);
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
    // copyPixels clips the source rect to the source BitmapData (0..width x 0..height).
    if (sheet.width > 0.0f && sheet.height > 0.0f) {
        const float ux0 = std::max(sx, 0.0f), uy0 = std::max(sy, 0.0f);
        const float ux1 = std::min(sx + sw, sheet.width), uy1 = std::min(sy + sh, sheet.height);
        dx += ux0 - sx;
        dy += uy0 - sy;
        sx = ux0;
        sy = uy0;
        sw = ux1 - ux0;
        sh = uy1 - uy0;
        if (sw <= 0.0f || sh <= 0.0f) {
            return;
        }
    }
    float x0 = (sx - sheet.originX) * sheet.scale, y0 = (sy - sheet.originY) * sheet.scale;
    float x1 = x0 + sw * sheet.scale, y1 = y0 + sh * sheet.scale;
    // Then to the texture (art outside the baked bounds is transparent); shift the destination to match.
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
