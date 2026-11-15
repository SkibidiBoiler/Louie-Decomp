#include "engine/import_screen.h"

#include <cstring>

namespace engine {

namespace {

// SKELETON: these texts move to the string table (data/lang) with the options menu.
constexpr const char *kTitle = "Import a game";
constexpr const char *kHint = "Pick a game, then choose your copy of its .swf file";

constexpr float kRowTop = 120.0f;
constexpr float kRowHeight = 56.0f;
constexpr float kRowMargin = 40.0f;

void drawText(SDL_Renderer *r, float x, float y, float scale, const char *text)
{
    SDL_SetRenderScale(r, scale, scale);
    SDL_RenderDebugText(r, x / scale, y / scale, text);
    SDL_SetRenderScale(r, 1.0f, 1.0f);
}

void drawCentered(SDL_Renderer *r, float cx, float y, float scale, const char *text)
{
    const float w = static_cast<float>(std::strlen(text)) * 8.0f * scale;
    drawText(r, cx - w / 2, y, scale, text);
}

} // namespace

ImportScreen::ImportScreen(SDL_Window *window, std::string dataDir) : window_(window), dataDir_(std::move(dataDir))
{
}

ImportScreen::~ImportScreen()
{
    importer_.cancel = true;
    if (thread_) {
        SDL_WaitThread(thread_, nullptr);
    }
}

bool ImportScreen::busy() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    return running_;
}

std::string ImportScreen::takeFinished()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::string id;
    id.swap(finished_);
    return id;
}

void ImportScreen::tap(float x, float y)
{
    if (busy()) {
        return;
    }
    int count = 0;
    swfimport::knownGames(count);
    for (int i = 0; i < count; ++i) {
        const float top = kRowTop + static_cast<float>(i) * kRowHeight;
        if (x >= kRowMargin && y >= top && y < top + kRowHeight - 8.0f) {
            chosenGame_ = i;
#if defined(SDL_PLATFORM_ANDROID)
            // The Android picker filters by MIME type, and .swf often has none registered: show every file.
            SDL_ShowOpenFileDialog(&ImportScreen::onFileChosen, this, window_, nullptr, 0, nullptr, false);
#else
            static const SDL_DialogFileFilter filters[] = {{"Flash games (*.swf)", "swf"}};
            SDL_ShowOpenFileDialog(&ImportScreen::onFileChosen, this, window_, filters, 1, nullptr, false);
#endif
            return;
        }
    }
}

void SDLCALL ImportScreen::onFileChosen(void *userdata, const char *const *files, int)
{
    auto *self = static_cast<ImportScreen *>(userdata);
    if (!files) {
        std::lock_guard<std::mutex> lock(self->mutex_);
        self->message_ = std::string("File picker error: ") + SDL_GetError();
        return;
    }
    if (!files[0]) {
        return; // cancelled
    }
    self->start(files[0]);
}

void ImportScreen::start(const std::string &path)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (running_) {
        return;
    }
    if (thread_) {
        SDL_WaitThread(thread_, nullptr);
        thread_ = nullptr;
    }
    path_ = path;
    running_ = true;
    message_.clear();
    progress_ = {};
    importer_.cancel = false;
    thread_ = SDL_CreateThread(&ImportScreen::worker, "import", this);
    if (!thread_) {
        running_ = false;
        message_ = std::string("Cannot start the import: ") + SDL_GetError();
    }
}

int SDLCALL ImportScreen::worker(void *userdata)
{
    auto *self = static_cast<ImportScreen *>(userdata);
    int count = 0;
    const swfimport::GameInfo *games = swfimport::knownGames(count);
    std::string path, expected;
    {
        std::lock_guard<std::mutex> lock(self->mutex_);
        path = self->path_;
        expected = self->chosenGame_ >= 0 && self->chosenGame_ < count ? games[self->chosenGame_].id : "";
    }
    SDL_CreateDirectory(self->dataDir_.c_str());
    const bool ok = self->importer_.run(path, self->dataDir_, [self](const swfimport::Progress &p) {
        std::lock_guard<std::mutex> lock(self->mutex_);
        self->progress_ = p;
    }, expected);
    std::lock_guard<std::mutex> lock(self->mutex_);
    self->running_ = false;
    if (ok) {
        self->finished_ = self->importer_.gameId();
        self->message_ = "Imported";
    } else {
        self->message_ = "Import failed: " + self->importer_.error();
    }
    SDL_Log("Import %s: %s", ok ? "done" : "failed", self->message_.c_str());
    return ok ? 0 : 1;
}

void ImportScreen::render(SDL_Renderer *r, float stageW, float stageH)
{
    SDL_SetRenderDrawColor(r, 34, 40, 58, 255);
    const SDL_FRect all{0, 0, stageW, stageH};
    SDL_RenderFillRect(r, &all);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    drawCentered(r, stageW / 2, 40, 3.0f, kTitle);
    drawCentered(r, stageW / 2, 84, 1.0f, kHint);

    std::lock_guard<std::mutex> lock(mutex_);
    int count = 0;
    const swfimport::GameInfo *games = swfimport::knownGames(count);
    for (int i = 0; i < count; ++i) {
        const float top = kRowTop + static_cast<float>(i) * kRowHeight;
        const SDL_FRect row{kRowMargin, top, stageW - 2 * kRowMargin, kRowHeight - 8.0f};
        const bool chosen = i == chosenGame_;
        SDL_SetRenderDrawColor(r, chosen ? 230 : 70, chosen ? 120 : 82, chosen ? 40 : 110, 255);
        SDL_RenderFillRect(r, &row);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        drawText(r, row.x + 16, row.y + (row.h - 16) / 2, 2.0f, games[i].title);
    }
    const float y = kRowTop + static_cast<float>(count) * kRowHeight + 16.0f;
    if (running_) {
        const SDL_FRect bar{kRowMargin, y, stageW - 2 * kRowMargin, 14};
        SDL_SetRenderDrawColor(r, 70, 82, 110, 255);
        SDL_RenderFillRect(r, &bar);
        SDL_FRect fill = bar;
        fill.w *= progress_.fraction;
        SDL_SetRenderDrawColor(r, 120, 210, 120, 255);
        SDL_RenderFillRect(r, &fill);
        SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
        drawCentered(r, stageW / 2, y + 24, 1.0f, progress_.step.c_str());
    } else if (!message_.empty()) {
        drawCentered(r, stageW / 2, y + 4, 1.0f, message_.c_str());
    }
}

} // namespace engine
