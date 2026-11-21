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
    auto it = silhouettes_.find(id);
    if (it != silhouettes_.end()) {
        return it->second;
    }
    SDL_Texture *texture = nullptr;
    const std::string path = dir_ + std::to_string(id) + ".png";
    if (SDL_Surface *loaded = SDL_LoadPNG(path.c_str())) {
        if (SDL_Surface *surface = SDL_ConvertSurface(loaded, SDL_PIXELFORMAT_RGBA32)) {
            for (int y = 0; y < surface->h; ++y) {
                uint8_t *row = static_cast<uint8_t *>(surface->pixels) + static_cast<size_t>(y) * surface->pitch;
                for (int x = 0; x < surface->w; ++x) {
                    row[x * 4] = row[x * 4 + 1] = row[x * 4 + 2] = 255; // R, G, B; alpha stays
                }
            }
            texture = SDL_CreateTextureFromSurface(renderer_, surface);
            SDL_DestroySurface(surface);
            if (texture) {
                SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_ADD); // dst += src * src alpha
            }
        }
        SDL_DestroySurface(loaded);
    }
    silhouettes_[id] = texture;
    return texture;
}

} // namespace engine
