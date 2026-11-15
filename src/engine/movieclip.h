#pragma once
// Runtime instance of a timeline symbol: its own playhead plus persistent child instances.
#include <SDL3/SDL.h>
#include <memory>
#include <vector>

#include "engine/math.h"
#include "engine/symbols.h"

namespace engine {

class ShapeTextures;

struct RenderContext {
    SDL_Renderer *renderer = nullptr;
    const SymbolLibrary *symbols = nullptr;
    ShapeTextures *textures = nullptr;
};

class MovieClip {
public:
    MovieClip(const SymbolLibrary &symbols, const Timeline &timeline);

    int currentFrame() const { return frame_; }
    int frameCount() const { return timeline_->frameCount; }
    bool playing() const { return playing_; }
    void play() { playing_ = true; }
    void stop() { playing_ = false; }
    void gotoFrame(int frame);

    // One Flash frame: move the playhead (looping), then advance the children that survived.
    void advance();

    void render(const RenderContext &ctx, const Affine &parent, const ColorTransform &parentColor) const;

    // Grows [minX,maxX]x[minY,maxY] by the visible shapes of the current frame, transformed by `parent`.
    void accumulateBounds(const Affine &parent, float &minX, float &minY, float &maxX, float &maxY) const;

private:
    struct Child {
        const Track *track = nullptr;
        std::unique_ptr<MovieClip> clip; // only for sprite children
    };

    void syncChildren(bool advanceSurvivors);

    const SymbolLibrary *symbols_;
    const Timeline *timeline_;
    int frame_ = 0;
    bool playing_ = true;
    std::vector<Child> children_; // sorted by depth
};

} // namespace engine
