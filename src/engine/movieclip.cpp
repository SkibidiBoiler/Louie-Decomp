#include "engine/movieclip.h"

#include "engine/text.h"
#include "engine/textures.h"

#include <algorithm>
#include <limits>

namespace engine {

namespace {

float clamp01(float v) { return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v); }

float lerp(float a, float b, float t) { return a + (b - a) * t; }

Affine lerp(const Affine &p, const Affine &q, float t)
{
    return {lerp(p.a, q.a, t), lerp(p.b, q.b, t), lerp(p.c, q.c, t),
            lerp(p.d, q.d, t), lerp(p.tx, q.tx, t), lerp(p.ty, q.ty, t)};
}

ColorTransform lerp(const ColorTransform &p, const ColorTransform &q, float t)
{
    return {lerp(p.mr, q.mr, t), lerp(p.mg, q.mg, t), lerp(p.mb, q.mb, t), lerp(p.ma, q.ma, t),
            lerp(p.ar, q.ar, t), lerp(p.ag, q.ag, t), lerp(p.ab, q.ab, t), lerp(p.aa, q.aa, t)};
}

void drawShape(const RenderContext &ctx, uint16_t id, const Affine &m, const ColorTransform &color)
{
    const ShapeBounds *bounds = ctx.symbols->shape(id);
    SDL_Texture *texture = ctx.textures->get(id);
    if (!bounds || !texture) {
        return;
    }
    float tw = 0.0f, th = 0.0f;
    SDL_GetTextureSize(texture, &tw, &th);
    const float scale = static_cast<float>(ctx.textures->scale());
    const float x0 = bounds->x, y0 = bounds->y;
    const float x1 = x0 + tw / scale, y1 = y0 + th / scale;

    // TODO: additive colour terms need a shader; only the multiplier is applied for now.
    const SDL_FColor tint{clamp01(color.mr), clamp01(color.mg), clamp01(color.mb), clamp01(color.ma)};
    SDL_Vertex v[4];
    const float corners[4][4] = {{x0, y0, 0, 0}, {x1, y0, 1, 0}, {x1, y1, 1, 1}, {x0, y1, 0, 1}};
    for (int i = 0; i < 4; ++i) {
        m.apply(corners[i][0], corners[i][1], v[i].position.x, v[i].position.y);
        v[i].color = tint;
        v[i].tex_coord = {corners[i][2], corners[i][3]};
    }
    static const int indices[6] = {0, 1, 2, 0, 2, 3};
    SDL_RenderGeometry(ctx.renderer, texture, v, 4, indices, 6);
}

// Bounds of the tracks of `tl` alive at `frame` (shapes, and the first frame of nested sprites / buttons).
void timelineBounds(const SymbolLibrary &symbols, const Timeline &tl, int frame, const Affine &parent, float &minX,
                    float &minY, float &maxX, float &maxY, int depth = 0)
{
    if (depth > 8) {
        return;
    }
    for (const Track &track : tl.tracks) {
        if (!track.aliveAt(frame) || track.clipDepth > 0) {
            continue;
        }
        const Affine m = parent * track.keyAt(frame).matrix;
        const CharKind kind = symbols.kind(track.charId);
        if (kind == CharKind::Sprite || kind == CharKind::Button) {
            if (const Timeline *child = symbols.timeline(track.charId)) {
                timelineBounds(symbols, *child, 0, m, minX, minY, maxX, maxY, depth + 1);
            }
        } else if (const ShapeBounds *b = symbols.shape(track.charId)) {
            const float xs[2] = {b->x, b->x + b->w};
            const float ys[2] = {b->y, b->y + b->h};
            for (float x : xs) {
                for (float y : ys) {
                    float px = 0.0f, py = 0.0f;
                    m.apply(x, y, px, py);
                    minX = std::min(minX, px);
                    minY = std::min(minY, py);
                    maxX = std::max(maxX, px);
                    maxY = std::max(maxY, py);
                }
            }
        }
    }
}

uint64_t gPointerTick = 1;

} // namespace

MaskLayers::~MaskLayers()
{
    if (content_) {
        SDL_DestroyTexture(content_);
    }
    if (mask_) {
        SDL_DestroyTexture(mask_);
    }
}

void MaskLayers::draw(SDL_Renderer *renderer, const std::function<void(const Affine &)> &content,
                      const std::function<void(const Affine &)> &mask)
{
    int lw = 0, lh = 0;
    SDL_RendererLogicalPresentation mode;
    SDL_GetRenderLogicalPresentation(renderer, &lw, &lh, &mode);
    SDL_FRect area;
    if (lw <= 0 || lh <= 0 || !SDL_GetRenderLogicalPresentationRect(renderer, &area)) {
        content(Affine{});
        return;
    }
    const int w = std::max(1, static_cast<int>(area.w)), h = std::max(1, static_cast<int>(area.h));
    if (w != w_ || h != h_ || !content_ || !mask_) {
        if (content_) {
            SDL_DestroyTexture(content_);
        }
        if (mask_) {
            SDL_DestroyTexture(mask_);
        }
        content_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
        mask_ = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, w, h);
        w_ = w;
        h_ = h;
        if (!content_ || !mask_) {
            content(Affine{});
            return;
        }
        // content *= mask alpha
        SDL_SetTextureBlendMode(mask_, SDL_ComposeCustomBlendMode(
                                           SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDOPERATION_ADD,
                                           SDL_BLENDFACTOR_ZERO, SDL_BLENDFACTOR_SRC_ALPHA, SDL_BLENDOPERATION_ADD));
        SDL_SetTextureBlendMode(content_, SDL_BLENDMODE_BLEND_PREMULTIPLIED);
    }
    // Stage coordinates -> texture pixels.
    const Affine toTexture = Affine{static_cast<float>(w) / static_cast<float>(lw), 0, 0,
                                    static_cast<float>(h) / static_cast<float>(lh), 0, 0};
    SDL_SetRenderTarget(renderer, mask_);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    mask(toTexture);
    SDL_SetRenderTarget(renderer, content_);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    content(toTexture);
    SDL_RenderTexture(renderer, mask_, nullptr, nullptr);
    SDL_SetRenderTarget(renderer, nullptr);
    const SDL_FRect stage{0, 0, static_cast<float>(lw), static_cast<float>(lh)};
    SDL_RenderTexture(renderer, content_, nullptr, &stage);
}

MovieClip::MovieClip(const SymbolLibrary &symbols, const Timeline &timeline)
    : symbols_(&symbols), timeline_(&timeline)
{
    syncChildren(false);
}

void MovieClip::gotoFrame(int frame)
{
    frame_ = std::clamp(frame, 0, timeline_->frameCount - 1);
    prevFrame_ = -1;
    syncChildren(false);
}

void MovieClip::advance()
{
    if (isButton_ && hoverTick_ != gPointerTick && frame_ != 0) {
        gotoFrame(0);
    }
    prevFrame_ = frame_;
    const int before = frame_;
    if (playing_ && timeline_->frameCount > 1) {
        frame_ = frame_ + 1;
        if (frame_ >= timeline_->frameCount) {
            frame_ = 0;
            prevFrame_ = -1; // looping is a jump: don't tween from the last frame back to the first
        }
    }
    syncChildren(true);
    for (Attached &a : attached_) {
        a.clip->advance();
    }
    if (frame_ != before) {
        runScript(0);
    }
}

void MovieClip::goTo(int frame, bool play)
{
    const int target = std::clamp(frame, 0, timeline_->frameCount - 1);
    playing_ = play;
    if (target == frame_) {
        return; // Flash does not re-run the script of the frame it is already on
    }
    frame_ = target;
    prevFrame_ = -1;
    syncChildren(false);
    runScript(0);
}

bool MovieClip::gotoLabel(std::string_view label, bool play)
{
    const int frame = SymbolLibrary::labelFrame(*timeline_, label);
    if (frame < 0) {
        return false;
    }
    goTo(frame, play);
    return true;
}

void MovieClip::runScript(int depth)
{
    const auto *ops = symbols_->frameScript(timeline_->id, frame_);
    if (!ops || depth > 8) {
        return;
    }
    const int frame = frame_;
    for (const FrameScriptOp &op : *ops) {
        switch (op.op) {
        case FrameScriptOp::Op::Stop: playing_ = false; break;
        case FrameScriptOp::Op::Play: playing_ = true; break;
        case FrameScriptOp::Op::GotoAndPlay:
        case FrameScriptOp::Op::GotoAndStop: {
            const bool play = op.op == FrameScriptOp::Op::GotoAndPlay;
            const int target = op.target.empty() ? op.number - 1 : SymbolLibrary::labelFrame(*timeline_, op.target);
            if (target >= 0) {
                goTo(target, play);
            }
            return; // the rest of this frame's script belongs to a frame we left
        }
        case FrameScriptOp::Op::Child: {
            // MovieClip(this.<target>.getChildAt(0)).gotoAndStop(n), inside the script's try block.
            MovieClip *container = findChild(op.target);
            MovieClip *part = container ? container->firstChildClip() : nullptr;
            if (!part) {
                return; // AS3 throws; the catch ends the try block, which holds the whole script
            }
            part->gotoAndStop(op.number);
            break;
        }
        }
        if (frame_ != frame) {
            return;
        }
    }
}

MovieClip *MovieClip::findChild(std::string_view name)
{
    for (Child &c : children_) {
        if (c.clip && c.track->name >= 0 &&
            static_cast<size_t>(c.track->name) < symbols_->strings().size() &&
            symbols_->strings()[static_cast<size_t>(c.track->name)] == name) {
            return c.clip.get();
        }
    }
    return nullptr;
}

bool MovieClip::childMatrix(std::string_view name, Affine &out) const
{
    for (const Child &c : children_) {
        if (c.track->name >= 0 && static_cast<size_t>(c.track->name) < symbols_->strings().size() &&
            symbols_->strings()[static_cast<size_t>(c.track->name)] == name) {
            out = c.track->keyAt(frame_).matrix;
            return true;
        }
    }
    return false;
}

void MovieClip::forEachChild(const std::function<void(const std::string &, MovieClip &)> &fn)
{
    static const std::string kUnnamed;
    for (Child &c : children_) {
        if (!c.clip) {
            continue;
        }
        const bool named = c.track->name >= 0 && static_cast<size_t>(c.track->name) < symbols_->strings().size();
        fn(named ? symbols_->strings()[static_cast<size_t>(c.track->name)] : kUnnamed, *c.clip);
    }
}

MovieClip &MovieClip::addChild(std::unique_ptr<MovieClip> clip, std::string name)
{
    attached_.push_back(Attached{std::move(name), std::move(clip)});
    return *attached_.back().clip;
}

MovieClip *MovieClip::firstChildClip()
{
    if (!children_.empty()) {
        return children_.front().clip.get(); // a shape at index 0 is not a MovieClip
    }
    return attached_.empty() ? nullptr : attached_.front().clip.get();
}

void MovieClip::syncChildren(bool advanceSurvivors)
{
    std::vector<Child> next;
    for (const Track &track : timeline_->tracks) {
        if (!track.aliveAt(frame_)) {
            continue;
        }
        auto it = std::find_if(children_.begin(), children_.end(),
                               [&](const Child &c) { return c.track == &track; });
        if (it != children_.end()) {
            if (advanceSurvivors && it->clip) {
                it->clip->advance();
            }
            next.push_back(std::move(*it));
            continue;
        }
        Child child;
        child.track = &track;
        const CharKind kind = symbols_->kind(track.charId);
        if (kind == CharKind::Sprite || kind == CharKind::Button) {
            if (const Timeline *tl = symbols_->timeline(track.charId)) {
                child.clip = std::make_unique<MovieClip>(*symbols_, *tl);
                if (kind == CharKind::Button) {
                    child.clip->isButton_ = true;
                    child.clip->gotoFrame(0);
                    child.clip->playing_ = false;
                }
            }
        }
        next.push_back(std::move(child));
    }
    std::stable_sort(next.begin(), next.end(),
                     [](const Child &a, const Child &b) { return a.track->depth < b.track->depth; });
    children_ = std::move(next);
}

void MovieClip::setChildAlpha(std::string_view instance, float alpha)
{
    for (auto &[name, value] : alphas_) {
        if (name == instance) {
            value = alpha;
            return;
        }
    }
    alphas_.emplace_back(std::string(instance), alpha);
}

void MovieClip::setText(std::string_view instance, std::string text)
{
    for (auto &[name, value] : texts_) {
        if (name == instance) {
            value = std::move(text);
            return;
        }
    }
    texts_.emplace_back(std::string(instance), std::move(text));
}

Affine MovieClip::childLocal(const Child &child, bool interpolate, float alpha) const
{
    const Track &track = *child.track;
    const TimelineKey &key = track.keyAt(frame_);
    Affine local = key.matrix;
    // Tween from the previous frame when this same instance was already on stage there.
    if (interpolate && prevFrame_ >= 0 && prevFrame_ != frame_ && track.aliveAt(prevFrame_)) {
        const TimelineKey &prev = track.keyAt(prevFrame_);
        if (&prev != &key) {
            local = lerp(prev.matrix, key.matrix, alpha);
        }
    }
    if (child.clip && child.clip->hasScale_) {
        local.a = child.clip->scaleX_;
        local.b = 0;
        local.c = 0;
        local.d = child.clip->scaleY_;
    }
    if (child.clip && child.clip->hasPos_) {
        local.tx = child.clip->x_;
        local.ty = child.clip->y_;
    }
    return local;
}

void MovieClip::renderChild(const RenderContext &ctx, const Child &child, const Affine &parent,
                            const ColorTransform &parentColor) const
{
    const Track &track = *child.track;
    const TimelineKey &key = track.keyAt(frame_);
    ColorTransform localColor = key.hasColor ? key.color : ColorTransform{};
    if (ctx.interpolate && prevFrame_ >= 0 && prevFrame_ != frame_ && track.aliveAt(prevFrame_)) {
        const TimelineKey &prev = track.keyAt(prevFrame_);
        if (&prev != &key) {
            localColor = lerp(prev.hasColor ? prev.color : ColorTransform{}, localColor, ctx.alpha);
        }
    }
    const Affine m = parent * childLocal(child, ctx.interpolate, ctx.alpha);
    ColorTransform color = parentColor * localColor;
    if (!alphas_.empty() && track.name >= 0 && static_cast<size_t>(track.name) < symbols_->strings().size()) {
        for (const auto &[name, value] : alphas_) {
            if (name == symbols_->strings()[static_cast<size_t>(track.name)]) {
                color.ma *= value;
                color.aa *= value;
            }
        }
    }
    if (color.ma <= 0.0f && color.aa <= 0.0f) {
        return;
    }
    if (child.clip) {
        if (child.clip->visible_) {
            child.clip->render(ctx, m, color);
        }
    } else if (ctx.symbols->kind(track.charId) == CharKind::Shape) {
        drawShape(ctx, track.charId, m, color);
    } else if (ctx.text && ctx.symbols->kind(track.charId) == CharKind::Text) {
        ctx.text->drawStatic(track.charId, m, color);
    } else if (ctx.text && ctx.symbols->kind(track.charId) == CharKind::EditText) {
        const std::string *text = nullptr;
        if (track.name >= 0 && static_cast<size_t>(track.name) < symbols_->strings().size()) {
            const std::string &instance = symbols_->strings()[static_cast<size_t>(track.name)];
            for (const auto &[name, value] : texts_) {
                if (name == instance) {
                    text = &value;
                }
            }
        }
        ctx.text->drawEdit(track.charId, text, m, color);
    }
    // TODO: morph shapes are not drawn yet.
}

void MovieClip::render(const RenderContext &ctx, const Affine &parent, const ColorTransform &parentColor) const
{
    // A frame-0 button shows its "up" state; the hit area (frame 3) is never drawn.
    for (size_t i = 0; i < children_.size(); ++i) {
        const Child &child = children_[i];
        const Track &track = *child.track;
        if (track.clipDepth == 0) {
            renderChild(ctx, child, parent, parentColor);
            continue;
        }
        // A mask: it clips the following children up to its clip depth.
        size_t last = i;
        while (last + 1 < children_.size() && children_[last + 1].track->depth <= track.clipDepth) {
            ++last;
        }
        const bool canMask = ctx.masks && SDL_GetRenderTarget(ctx.renderer) == nullptr;
        if (!canMask) {
            for (size_t j = i + 1; j <= last; ++j) {
                renderChild(ctx, children_[j], parent, parentColor);
            }
        } else {
            RenderContext inner = ctx;
            ctx.masks->draw(
                ctx.renderer,
                [&](const Affine &toTexture) {
                    for (size_t j = i + 1; j <= last; ++j) {
                        renderChild(inner, children_[j], toTexture * parent, parentColor);
                    }
                },
                [&](const Affine &toTexture) {
                    // The mask's shape counts, not its colour or alpha.
                    renderChild(inner, child, toTexture * parent, ColorTransform{});
                });
        }
        i = last;
    }
    for (const Attached &a : attached_) {
        if (a.clip->visible_) {
            Affine m = parent;
            if (a.clip->hasPos_ || a.clip->hasScale_) {
                Affine local;
                if (a.clip->hasScale_) {
                    local.a = a.clip->scaleX_;
                    local.d = a.clip->scaleY_;
                }
                if (a.clip->hasPos_) {
                    local.tx = a.clip->x_;
                    local.ty = a.clip->y_;
                }
                m = parent * local;
            }
            a.clip->render(ctx, m, parentColor);
        }
    }
}

MovieClip *MovieClip::hitTest(const Affine &parent, float px, float py)
{
    if (!visible_ || !mouseEnabled) {
        return nullptr;
    }
    auto inside = [&](float minX, float minY, float maxX, float maxY) {
        return px >= minX && px <= maxX && py >= minY && py <= maxY;
    };
    constexpr float kBig = std::numeric_limits<float>::max();
    if (isButton_) {
        float minX = kBig, minY = kBig, maxX = -kBig, maxY = -kBig;
        timelineBounds(*symbols_, *timeline_, 3, parent, minX, minY, maxX, maxY);
        return inside(minX, minY, maxX, maxY) ? this : nullptr;
    }
    // Topmost first: attached clips, then the timeline children from the highest depth.
    for (auto it = attached_.rbegin(); it != attached_.rend(); ++it) {
        Affine m = parent;
        if (it->clip->hasPos_) {
            m = parent * Affine::translate(it->clip->x_, it->clip->y_);
        }
        if (MovieClip *hit = it->clip->hitTest(m, px, py)) {
            return hit;
        }
    }
    for (auto it = children_.rbegin(); it != children_.rend(); ++it) {
        if (it->clip && it->track->clipDepth == 0) {
            if (MovieClip *hit = it->clip->hitTest(parent * childLocal(*it, false, 1.0f), px, py)) {
                return hit;
            }
        }
    }
    if (hasHandler()) {
        float minX = kBig, minY = kBig, maxX = -kBig, maxY = -kBig;
        accumulateBounds(parent, minX, minY, maxX, maxY);
        if (inside(minX, minY, maxX, maxY)) {
            return this;
        }
    }
    return nullptr;
}

void MovieClip::dispatchPointer(MovieClip &root, const Affine &parent, const PointerState &pointer)
{
    static MovieClip *lastOver = nullptr;    // compared only, never dereferenced (it may be gone)
    static MovieClip *pressedOn = nullptr;   // the same
    ++gPointerTick;
    MovieClip *target = pointer.x >= 0 ? root.hitTest(parent, pointer.x, pointer.y) : nullptr;
    if (target) {
        target->hoverTick_ = gPointerTick;
        target->hoverDown_ = pointer.down;
        if (target->isButton_) {
            target->gotoFrame(pointer.down ? 2 : 1);
        }
    }
    if (target != lastOver && target && target->onRollOver) {
        target->onRollOver();
    }
    lastOver = target;
    if (pointer.pressed) {
        pressedOn = target;
        if (target && target->onMouseDown) {
            target->onMouseDown();
        }
    }
    if (pointer.released) {
        MovieClip *was = pressedOn;
        pressedOn = nullptr;
        if (target && target == was && target->onClick) {
            target->onClick();
        }
    }
}

void MovieClip::accumulateBounds(const Affine &parent, float &minX, float &minY, float &maxX, float &maxY) const
{
    for (const Child &child : children_) {
        const Track &track = *child.track;
        if (track.clipDepth > 0) {
            continue;
        }
        const TimelineKey &key = track.keyAt(frame_);
        if (key.hasColor && key.color.ma <= 0.0f && key.color.aa <= 0.0f) {
            continue;
        }
        const Affine m = parent * childLocal(child, false, 1.0f);
        if (child.clip) {
            if (child.clip->visible_) {
                child.clip->accumulateBounds(m, minX, minY, maxX, maxY);
            }
        } else if (const ShapeBounds *b = symbols_->shape(track.charId)) {
            const float xs[2] = {b->x, b->x + b->w};
            const float ys[2] = {b->y, b->y + b->h};
            for (float x : xs) {
                for (float y : ys) {
                    float px = 0.0f, py = 0.0f;
                    m.apply(x, y, px, py);
                    minX = std::min(minX, px);
                    minY = std::min(minY, py);
                    maxX = std::max(maxX, px);
                    maxY = std::max(maxY, py);
                }
            }
        }
    }
    for (const Attached &a : attached_) {
        if (a.clip->visible_) {
            a.clip->accumulateBounds(parent, minX, minY, maxX, maxY);
        }
    }
}

std::unique_ptr<MovieClip> MovieClip::create(const SymbolLibrary &symbols, std::string_view className)
{
    const int id = symbols.findClass(className);
    const Timeline *tl = id >= 0 ? symbols.timeline(static_cast<uint16_t>(id)) : nullptr;
    return tl ? std::make_unique<MovieClip>(symbols, *tl) : nullptr;
}

std::unique_ptr<MovieClip> MovieClip::createEmpty(const SymbolLibrary &symbols)
{
    static const Timeline empty{};
    return std::make_unique<MovieClip>(symbols, empty);
}

void MovieClip::removeChild(const MovieClip *clip)
{
    attached_.erase(std::remove_if(attached_.begin(), attached_.end(),
                                   [&](const Attached &a) { return a.clip.get() == clip; }),
                    attached_.end());
}

} // namespace engine
