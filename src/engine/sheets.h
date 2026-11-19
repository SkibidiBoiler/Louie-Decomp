#pragma once
// Sheets: the bitmaps the original builds at runtime (BitmapManager), baked offline at N x by
// tools/swf2assets.py. Drawing mirrors the original's BitmapData.copyPixels, in sheet/stage units.
#include <SDL3/SDL.h>
#include <string>
#include <unordered_map>

namespace engine {

struct Sheet {
    SDL_Texture *texture = nullptr;
    float originX = 0.0f; // sheet units of the texture's top-left pixel
    float originY = 0.0f;
    float scale = 1.0f;   // texture pixels per sheet unit
    float width = 0.0f;   // logical BitmapData size in sheet units (0 = texture size)
    float height = 0.0f;
    bool premultiplied = false;
};

class SheetLibrary {
public:
    SheetLibrary() = default;
    SheetLibrary(const SheetLibrary &) = delete;
    SheetLibrary &operator=(const SheetLibrary &) = delete;
    ~SheetLibrary() { clear(); }

    // Reads <gameDir>/sheets/index.txt. `scale` is the preferred vector scale.
    bool init(SDL_Renderer *renderer, const std::string &gameDir, int scale);
    void clear();

    // Loads on first use; nullptr if the sheet does not exist.
    const Sheet *get(const std::string &name);

    // Equivalent of BitmapManager.createTilesheet: the five parts of tileset `name` stacked into one
    // 1984x1312 sheet (each part clipped to its band). Cached under `name`.
    const Sheet *composeTilesheet(const std::string &name);

    // copyPixels(sheet, Rectangle(sx, sy, sw, sh), Point(dx, dy)) onto the current target, in stage units.
    void blit(const Sheet &sheet, float sx, float sy, float sw, float sh, float dx, float dy) const;

private:
    struct Entry {
        float originX = 0.0f, originY = 0.0f;
        float width = 0.0f, height = 0.0f;
        bool bitmap = false;
    };

    SDL_Renderer *renderer_ = nullptr;
    std::string dir_;
    int scale_ = 1;
    std::unordered_map<std::string, Entry> index_;
    std::unordered_map<std::string, Sheet> loaded_;
};

} // namespace engine
