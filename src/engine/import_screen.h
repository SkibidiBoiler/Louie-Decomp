#pragma once
// Port screen: pick a game, pick its .swf with the system file picker, convert it on a worker thread.
#include <SDL3/SDL.h>
#include <memory>
#include <mutex>
#include <string>

#include "import/importer.h"

namespace engine {

class ImportScreen {
public:
    ImportScreen(SDL_Window *window, std::string dataDir);
    ~ImportScreen();
    ImportScreen(const ImportScreen &) = delete;
    ImportScreen &operator=(const ImportScreen &) = delete;

    // A tap/click at stage coordinates.
    void tap(float x, float y);
    void render(SDL_Renderer *renderer, float stageW, float stageH);

    // Id of a game that finished importing since the last call ("" if none).
    std::string takeFinished();
    bool busy() const;

private:
    static void SDLCALL onFileChosen(void *userdata, const char *const *files, int filter);
    static int SDLCALL worker(void *userdata);
    void start(const std::string &path);

    SDL_Window *window_;
    std::string dataDir_;
    int chosenGame_ = -1;

    mutable std::mutex mutex_;
    bool running_ = false;
    swfimport::Progress progress_;
    std::string message_;  // last result or error
    std::string finished_; // game id waiting for takeFinished()
    std::string path_;
    SDL_Thread *thread_ = nullptr;
    swfimport::Importer importer_;
};

} // namespace engine
