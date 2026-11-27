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

private:
    SDL_Renderer *renderer_ = nullptr;
    std::string dir_;
    int scale_ = 1;
    std::unordered_map<uint16_t, SDL_Texture *> cache_;
    std::unordered_map<uint16_t, SDL_Texture *> silhouettes_;
    std::unordered_map<uint16_t, SDL_Texture *> grays_;
    SDL_Texture *derived(uint16_t id, std::unordered_map<uint16_t, SDL_Texture *> &cache, bool gray);
};

} // namespace engine
