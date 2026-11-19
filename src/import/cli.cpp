// Desktop command line for the in-app importer: pl_import --swf <file> --out <dir>
// Same conversion the game runs on the device; useful for checks and comparisons with tools/swf2assets.py.
#include <cstdio>
#include <cstring>
#include <string>

#include "import/importer.h"

int main(int argc, char **argv)
{
    std::string swf, out;
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::strcmp(argv[i], "--swf") == 0) {
            swf = argv[++i];
        } else if (std::strcmp(argv[i], "--out") == 0) {
            out = argv[++i];
        }
    }
    if (swf.empty() || out.empty()) {
        std::fprintf(stderr, "usage: pl_import --swf <game.swf> --out <dir>\n");
        return 2;
    }
    swfimport::Importer importer;
    const bool ok = importer.run(swf, out, [](const swfimport::Progress &p) {
        std::printf("[%3d%%] %s\n", static_cast<int>(p.fraction * 100.0f + 0.5f), p.step.c_str());
    });
    if (!ok) {
        std::fprintf(stderr, "error: %s\n", importer.error().c_str());
        return 1;
    }
    std::printf("done: %s (%s)\n", importer.gameId().c_str(), out.c_str());
    return 0;
}
