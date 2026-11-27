#include "engine/textures.h"

namespace engine {

void ShapeTextures::clear()
{
    for (auto &entry : cache_) {
        if (entry.second) {
            SDL_DestroyTexture(entry.second);
        }
    }
    cache_.clear();
    for (auto &entry : silhouettes_) {
        if (entry.second) {
            SDL_DestroyTexture(entry.second);
        }
    }
    silhouettes_.clear();
    for (auto &entry : grays_) {
        if (entry.second) {
            SDL_DestroyTexture(entry.second);
        }
    }
    grays_.clear();
}

void ShapeTextures::init(SDL_Renderer *renderer, const std::string &gameDir, int scale)
{
    renderer_ = renderer;
    scale_ = scale;
    dir_ = gameDir + "/shapes/" + std::to_string(scale) + "x/";
}

SDL_Texture *ShapeTextures::get(uint16_t id)
{
    auto it = cache_.find(id);
    if (it != cache_.end()) {
        return it->second;
    }
    SDL_Texture *texture = nullptr;
    const std::string path = dir_ + std::to_string(id) + ".png";
    if (SDL_Surface *surface = SDL_LoadPNG(path.c_str())) {
        texture = SDL_CreateTextureFromSurface(renderer_, surface);
        SDL_DestroySurface(surface);
        if (texture) {
            SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        }
    }
    if (!texture) {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "Shape %u: %s", id, SDL_GetError());
    }
    cache_[id] = texture;
    return texture;
}

SDL_Texture *ShapeTextures::silhouette(uint16_t id)
{
    return derived(id, silhouettes_, false);
}

SDL_Texture *ShapeTextures::grayscale(uint16_t id)
{
    return derived(id, grays_, true);
}

// A copy of the shape's PNG with its colours replaced: white (silhouette, drawn additively) or grey.
SDL_Texture *ShapeTextures::derived(uint16_t id, std::unordered_map<uint16_t, SDL_Texture *> &cache, bool gray)
{
    auto it = cache.find(id);
    if (it != cache.end()) {
        return it->second;
    }
    SDL_Texture *texture = nullptr;
    const std::string path = dir_ + std::to_string(id) + ".png";
    if (SDL_Surface *loaded = SDL_LoadPNG(path.c_str())) {
        if (SDL_Surface *surface = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32)) {
            for (int y = 0; y < surface->h; ++y) {
                uint8_t *row = static_cast<uint8_t *>(surface->pixels) + static_cast<size_t>(y) * surface->pitch;
                for (int x = 0; x < surface->w; ++x) {
                    uint8_t *px = row + x * 4;
                    const uint8_t v = gray ? static_cast<uint8_t>(0.212671 * px[0] + 0.71516 * px[1] + 0.072169 * px[2])
                                           : 255;
                    px[0] = px[1] = px[2] = v; // alpha stays
                }
            }
            texture = SDL_CreateTextureFromSurface(renderer_, surface);
            SDL_DestroySurface(surface);
            if (texture) {
                SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
                SDL_SetTextureBlendMode(texture, gray ? SDL_BLENDMODE_BLEND : SDL_BLENDMODE_ADD); // ADD: dst += src * a
            }
        }
        SDL_DestroySurface(loaded);
    }
    cache[id] = texture;
    return texture;
}

} // namespace engine
