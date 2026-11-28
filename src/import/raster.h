#pragma once
// Rasterises SWF vector art for the importer (PlutoVG): every shape at N x, and the "sheets" (sprite frames
// and bitmaps the game turns into BitmapData at runtime). Output: PNG files in the port's formats.
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "import/swf.h"
#include "import/symbols_export.h"

typedef struct plutovg_canvas plutovg_canvas_t;
typedef struct plutovg_surface plutovg_surface_t;

namespace swfimport {

struct Bounds {
    float x0 = 1e30f, y0 = 1e30f, x1 = -1e30f, y1 = -1e30f;
    bool empty() const { return x1 < x0 || y1 < y0; }
    void add(float x, float y)
    {
        x0 = x < x0 ? x : x0;
        y0 = y < y0 ? y : y0;
        x1 = x > x1 ? x : x1;
        y1 = y > y1 ? y : y1;
    }
};

class Rasterizer {
public:
    Rasterizer(const Swf &swf, const SwfIndex &index);
    ~Rasterizer();
    Rasterizer(const Rasterizer &) = delete;
    Rasterizer &operator=(const Rasterizer &) = delete;

    // shapes/<scale>x/<id>.png for every shape. `tick` is called after each shape (for progress/cancel);
    // returning false stops.
    bool exportShapes(const std::string &dir, int scale, const std::function<bool(size_t, size_t)> &tick);
    // shapes/<scale>x/m<id>_<ratio>.png for each morph step in `uses`, and shapes/<scale>x/morphs.txt
    // ("id ratio x y": each PNG's top-left in stage pixels).
    bool exportMorphs(const std::string &dir, int scale, const std::vector<std::pair<uint16_t, uint16_t>> &uses);

    // sheets/<scale>x/<name>.png (vector) or sheets/1x/<name>.(png) (bitmap) plus sheets/index.txt.
    bool exportSheets(const std::string &dir, const std::vector<std::string> &names, int scale,
                      const std::function<bool(size_t, size_t)> &tick);

    // fonts/<scale>x/<font id>.png (each font's glyphs, a 64 px em at 1x) and text.json: the fonts' glyph tables,
    // the static texts' glyph runs and the edit texts' boxes, styles and initial text.
    bool exportText(const std::string &dir, int scale);

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

} // namespace swfimport
