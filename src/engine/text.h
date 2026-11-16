#pragma once
// Text: the SWF's embedded fonts (glyph atlases baked by the importer, fonts/<N>x/<id>.png) and its static and
// edit text characters (text.json, docs/asset-format.md). Draws glyph quads through the current transform.
#include <SDL3/SDL.h>
#include <cstdint>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "engine/math.h"

namespace engine {

class TextLibrary {
public:
    TextLibrary() = default;
    TextLibrary(const TextLibrary &) = delete;
    TextLibrary &operator=(const TextLibrary &) = delete;
    ~TextLibrary() { clear(); }

    // Reads <gameDir>/text.json; false (and nothing drawn) when it is missing.
    bool init(SDL_Renderer *renderer, const std::string &gameDir);
    void clear();

    // A DefineText character, through `m`.
    void drawStatic(uint16_t id, const Affine &m, const ColorTransform &color);
    // A DefineEditText character showing `text` (its initial text when null); HTML when the field is an HTML
    // field or the text starts with '<' (the original sets htmlText).
    void drawEdit(uint16_t id, const std::string *text, const Affine &m, const ColorTransform &color);
    // The port's own interface text (menus, the translation tables) in an embedded font found by name: one line
    // at (x, baseline), align 0 left, 1 right, 2 center. Returns its width. Letters the font lacks (accents, ñ,
    // ¿ ¡) are built from the base letter and a mark the font has. Nothing is drawn when `draw` is false.
    float drawString(std::string_view fontName, float size, std::string_view utf8, float x, float baseline,
                     SDL_FColor color, int align, const Affine &m, bool draw = true);

private:
    struct Glyph {
        uint32_t code = 0;
        float advance = 0;          // em units (em = em_ px)
        float x = 0, y = 0, w = 0, h = 0, ox = 0, oy = 0; // atlas pixels
    };
    struct Font {
        std::string name;
        bool bold = false, italic = false;
        float ascent = 0, descent = 0, leading = 0; // em units
        std::vector<Glyph> glyphs;
        std::unordered_map<uint32_t, int> byCode;
        SDL_Texture *texture = nullptr;
        bool loaded = false;
    };
    struct Run {
        uint16_t font = 0;
        float height = 0;
        SDL_FColor color{0, 0, 0, 1};
        float x = 0, y = 0;
        std::vector<std::pair<int, float>> glyphs; // glyph index, advance (px)
    };
    struct Static {
        Affine matrix;
        std::vector<Run> runs;
    };
    struct Edit {
        float x0 = 0, y0 = 0, x1 = 0, y1 = 0;
        uint16_t font = 0;
        float height = 12;
        SDL_FColor color{0, 0, 0, 1};
        int align = 0; // 0 left, 1 right, 2 center, 3 justify
        float left = 0, right = 0, indent = 0, leading = 0;
        bool wordWrap = false, multiline = false, html = false;
        std::string text;
    };
    // The style a piece of HTML text asks for.
    struct Style {
        int align = -1;
        float size = -1;
        bool bold = false;
        bool hasColor = false;
        SDL_FColor color{0, 0, 0, 1};
        std::string face;
    };

    Font *font(uint16_t id);
    Font *variant(uint16_t id, const std::string &face, bool bold);
    Font *fontByName(std::string_view name);
    void drawGlyph(const Font &f, const Glyph &g, float penX, float baseline, float size, const Affine &m,
                   SDL_FColor color);
    static std::string parseHtml(std::string_view html, Style &style);

    SDL_Renderer *renderer_ = nullptr;
    std::string dir_;
    float em_ = 64;
    float atlasScale_ = 1;
    std::unordered_map<uint16_t, Font> fonts_;
    std::unordered_map<uint16_t, Static> statics_;
    std::unordered_map<uint16_t, Edit> edits_;
};

} // namespace engine
