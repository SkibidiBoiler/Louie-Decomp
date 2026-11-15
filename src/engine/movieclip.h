#pragma once
// Runtime instance of a timeline symbol: its own playhead plus persistent child instances.
#include <SDL3/SDL.h>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "engine/math.h"
#include "engine/symbols.h"

namespace engine {

class ShapeTextures;
class TextLibrary;

struct RenderContext {
    SDL_Renderer *renderer = nullptr;
    const SymbolLibrary *symbols = nullptr;
    ShapeTextures *textures = nullptr;
    TextLibrary *text = nullptr; // static and edit texts; not drawn when null
    // Interpolation between the previous and the current logic step (0 = previous, 1 = current).
    bool interpolate = false;
    float alpha = 1.0f;
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

    // AS3 semantics (1-based frames); the target frame's converted script runs, as in Flash.
    void gotoAndStop(int frame1) { goTo(frame1 - 1, false); }
    void gotoAndPlay(int frame1) { goTo(frame1 - 1, true); }
    bool gotoLabel(std::string_view label, bool play);

    const Timeline &timeline() const { return *timeline_; }
    bool visible() const { return visible_; }
    void setVisible(bool v) { visible_ = v; }

    // Timeline child sprite with this instance name (current frame), or nullptr.
    MovieClip *findChild(std::string_view name);
    // Transform of that child in this clip's space; false if there is no such child.
    bool childMatrix(std::string_view name, Affine &out) const;
    // Calls fn(instance name, clip) for every timeline sprite child of the current frame.
    void forEachChild(const std::function<void(const std::string &, MovieClip &)> &fn);
    // addChild: a runtime child drawn above the timeline children, with an identity transform.
    MovieClip &addChild(std::unique_ptr<MovieClip> clip, std::string name);
    // getChildAt(0) as a MovieClip; nullptr when that child is not a MovieClip (AS3 would throw).
    MovieClip *firstChildClip();
    // `<instance>.text` / `.htmlText` of a timeline text field child (HTML when it starts with '<').
    void setText(std::string_view instance, std::string text);
    // `<instance>.alpha` of a timeline child that is not a MovieClip (a text field, a shape).
    void setChildAlpha(std::string_view instance, float alpha);

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
    void goTo(int frame, bool play);
    void runScript(int depth);

    const SymbolLibrary *symbols_;
    const Timeline *timeline_;
    int frame_ = 0;
    int prevFrame_ = -1; // frame shown one logic step ago, -1 after a jump/loop or on creation
    bool playing_ = true;
    std::vector<Child> children_; // sorted by depth
    struct Attached {
        std::string name;
        std::unique_ptr<MovieClip> clip;
    };
    std::vector<Attached> attached_;
    std::vector<std::pair<std::string, std::string>> texts_; // text field instance -> its text
    std::vector<std::pair<std::string, float>> alphas_;      // instance -> alpha set from code
    bool visible_ = true;
    int scriptDepth_ = 0;
};

} // namespace engine
