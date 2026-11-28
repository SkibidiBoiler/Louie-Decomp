#pragma once
// Shape textures, loaded on first use from shapes/<scale>x/<id>.png.
#include <SDL3/SDL.h>
#include <cstdint>
#include <string>
#include <unordered_map>

namespace engine {

class ShapeTextures {
public:
    ShapeTextures() = default;
    ShapeTextures(const ShapeTextures &) = delete;
    ShapeTextures &operator=(const ShapeTextures &) = delete;
    ~ShapeTextures() { clear(); }

    // Destroys every cached texture (call before destroying the renderer).
    void clear();
    void init(SDL_Renderer *renderer, const std::string &gameDir, int scale);
    int scale() const { return scale_; }

    // nullptr when the PNG is missing or fails to load (reported once).
    SDL_Texture *get(uint16_t id);
    // The same shape as a white silhouette (its alpha kept), drawn with additive blending: the additive terms of a
    // colour transform (a shape tinted white, for example). Made on first use.
    SDL_Texture *silhouette(uint16_t id);
    // The same shape desaturated with the luminance weights of the game's grey filter (0.2127, 0.7152, 0.0722).
    SDL_Texture *grayscale(uint16_t id);
    // A morph shape at one ratio: shapes/<scale>x/m<id>_<ratio>.png, placed at (x, y) in the shape's own space
    // (morphs.txt). nullptr when that step was not exported.
    SDL_Texture *morph(uint16_t id, uint16_t ratio, float &x, float &y);

private:
    SDL_Renderer *renderer_ = nullptr;
    std::string dir_;
    int scale_ = 1;
    std::unordered_map<uint16_t, SDL_Texture *> cache_;
    std::unordered_map<uint16_t, SDL_Texture *> silhouettes_;
    std::unordered_map<uint16_t, SDL_Texture *> grays_;
    std::unordered_map<uint32_t, SDL_Texture *> morphs_;            // (id << 16) | ratio
    std::unordered_map<uint32_t, std::pair<float, float>> morphAt_; // (id << 16) | ratio -> top-left
    SDL_Texture *derived(uint16_t id, std::unordered_map<uint16_t, SDL_Texture *> &cache, bool gray);
};

} // namespace engine
