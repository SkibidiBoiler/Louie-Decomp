#pragma once
// Drawing helpers for the port's own interface (options menu, game picker): flat translucent shapes in window
// pixels, in the style of the on-screen touch buttons.
#include <SDL3/SDL.h>

namespace engine::ui {

// The translucent panel the port's menus share.
inline constexpr SDL_FColor kPanelFill{0.05f, 0.07f, 0.12f, 0.62f};
inline constexpr SDL_FColor kPanelBorder{1, 1, 1, 0.55f};
inline constexpr SDL_FColor kTextBright{1, 1, 1, 0.95f};
inline constexpr SDL_FColor kTextDim{1, 1, 1, 0.7f};
inline constexpr SDL_FColor kRowHighlight{1, 1, 1, 0.14f};
inline constexpr SDL_FColor kRowSeparator{1, 1, 1, 0.18f};
inline constexpr const char *kFont = "Arial Black"; // an embedded font of PL2 and PL3

void fillRect(SDL_Renderer *r, float x, float y, float w, float h, SDL_FColor c);
void frameRect(SDL_Renderer *r, float x, float y, float w, float h, float t, SDL_FColor c);
void disc(SDL_Renderer *r, float cx, float cy, float radius, SDL_FColor c);
void ring(SDL_Renderer *r, float cx, float cy, float radius, float thickness, SDL_FColor c);
// The panel: fill and border (border thickness from the row height).
void panel(SDL_Renderer *r, float x, float y, float w, float h, float rowH);

} // namespace engine::ui
