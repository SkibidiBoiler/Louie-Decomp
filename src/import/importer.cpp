#include "import/importer.h"

#include <SDL3/SDL.h>

#include <cstdio>
#include <vector>

#include "import/abc.h"
#include "import/raster.h"
#include "import/sha256.h"
#include "import/swf.h"
#include "import/symbols_export.h"
#include "sheet_lists.h"

namespace swfimport {

namespace {

const GameInfo kGames[] = {
    {"pl1", "Papa Louie: When Pizzas Attack!", "666e2e935ccecbc7"},
    {"pl2", "Papa Louie 2: When Burgers Attack!", "8b3ac80fe6dbbaac"},
    {"pl3", "Papa Louie 3: When Sundaes Attack!", "3a349ecf7b874b89"},
};

bool writeFile(const std::string &path, const void *data, size_t size)
{
    return SDL_SaveFile(path.c_str(), data, size);
}

// JSON for a constant read from the bytecode (numbers, strings, arrays).
void appendJson(std::string &out, const AbcValue &v)
{
    switch (v.type) {
    case AbcValue::Type::Number: {
        char buf[32];
        if (v.number == static_cast<double>(static_cast<long long>(v.number))) {
            SDL_snprintf(buf, sizeof(buf), "%lld", static_cast<long long>(v.number));
        } else {
            SDL_snprintf(buf, sizeof(buf), "%.17g", v.number);
        }
        out += buf;
        break;
    }
    case AbcValue::Type::String:
        out += '"';
        for (char c : v.string) {
            if (c == '"' || c == '\\') {
                out += '\\';
                out += c;
            } else if (static_cast<unsigned char>(c) < 0x20) {
                char buf[8];
                SDL_snprintf(buf, sizeof(buf), "\\u%04x", static_cast<unsigned>(c));
                out += buf;
            } else {
                out += c;
            }
        }
        out += '"';
        break;
    case AbcValue::Type::Array:
        out += '[';
        for (size_t i = 0; i < v.items.size(); ++i) {
            if (i) {
                out += ',';
            }
            appendJson(out, v.items[i]);
        }
        out += ']';
        break;
    default:
        out += "null";
        break;
    }
}

// Class names of the sheets to bake for a game (tools/sheets/<game>.txt, compiled in).
std::vector<std::string> sheetList(const std::string &game)
{
    std::vector<std::string> names;
    if (game != "pl2") {
        return names; // SKELETON: sheet lists for PL3 and PL1 come with their ports
    }
    std::string line;
    for (const char *c = kSheetsPl2;; ++c) {
        if (*c == '\n' || *c == '\r' || *c == '\0') {
            if (!line.empty() && line[0] != '#') {
                names.push_back(line);
            }
            line.clear();
            if (*c == '\0') {
                break;
            }
        } else if (*c != ' ' && *c != '\t') {
            line += *c;
        }
    }
    return names;
}

// Level XML hidden in the pixels of a palette bitmap (papaGame.data.LevelData.prepareXML), decoded with the
// colour -> text tables read from the SWF's own bytecode. Returns "" when the game has no such data.
std::string decodeLevels(const SwfIndex &index, std::string &note)
{
    std::map<std::string, AbcValue> fields;
    for (const Tag &tag : index.abc) {
        size_t p = 0;
        if (tag.code == 82) { // DoABC2: flags, name
            p = 4;
            cstring(tag.body, tag.size, p);
        }
        Abc abc;
        if (!abc.parse(tag.body + p, tag.size - p)) {
            continue;
        }
        fields = abc.instanceFieldConstants("papaGame.data", "LevelData");
        if (!fields.empty()) {
            break;
        }
    }
    const auto name = fields.find("encodedBitmapName");
    const auto colors = fields.find("colorTable");
    const auto texts = fields.find("textTable");
    if (name == fields.end() || colors == fields.end() || texts == fields.end() ||
        name->second.type != AbcValue::Type::String || colors->second.type != AbcValue::Type::Array ||
        texts->second.type != AbcValue::Type::Array) {
        note = "LevelData tables not found";
        return {};
    }
    const auto cls = index.classes.find(name->second.string);
    if (cls == index.classes.end()) {
        note = "level bitmap not found";
        return {};
    }
    const auto def = index.defs.find(cls->second);
    if (def == index.defs.end() || (def->second.code != 20 && def->second.code != 36)) {
        note = "level bitmap is not lossless";
        return {};
    }
    const Tag &tag = def->second;
    const uint8_t format = tag.body[2];
    const uint16_t w = u16(tag.body + 3), h = u16(tag.body + 5);
    std::vector<uint32_t> pixels;
    if (format == 3) { // palette: RGB(A) table, then row-padded indices
        const size_t count = static_cast<size_t>(tag.body[7]) + 1;
        const std::vector<uint8_t> raw = inflate(tag.body + 8, tag.size - 8);
        const size_t step = tag.code == 20 ? 3 : 4;
        const size_t stride = (static_cast<size_t>(w) + 3) & ~static_cast<size_t>(3);
        if (raw.size() < step * count + stride * h) {
            note = "level bitmap truncated";
            return {};
        }
        std::vector<uint32_t> palette(count);
        for (size_t i = 0; i < count; ++i) {
            const uint8_t *c = &raw[step * i];
            palette[i] = (static_cast<uint32_t>(c[0]) << 16) | (static_cast<uint32_t>(c[1]) << 8) | c[2];
        }
        const uint8_t *data = raw.data() + step * count;
        pixels.reserve(static_cast<size_t>(w) * h);
        for (size_t y = 0; y < h; ++y) {
            for (size_t x = 0; x < w; ++x) {
                const uint8_t i = data[y * stride + x];
                pixels.push_back(i < count ? palette[i] : 0xFFFFFFFFu);
            }
        }
    } else if (format == 5) { // 32-bit (A/X)RGB
        const std::vector<uint8_t> raw = inflate(tag.body + 7, tag.size - 7);
        if (raw.size() < 4u * w * h) {
            note = "level bitmap truncated";
            return {};
        }
        for (size_t i = 0; i < static_cast<size_t>(w) * h; ++i) {
            pixels.push_back((static_cast<uint32_t>(raw[4 * i + 1]) << 16) | (static_cast<uint32_t>(raw[4 * i + 2]) << 8) |
                             raw[4 * i + 3]);
        }
    } else {
        note = "unsupported level bitmap format";
        return {};
    }
    std::map<uint32_t, size_t> lookup; // first index wins, like Array.indexOf
    const auto &colorItems = colors->second.items;
    for (size_t i = 0; i < colorItems.size(); ++i) {
        lookup.emplace(static_cast<uint32_t>(colorItems[i].number), i);
    }
    const auto &textItems = texts->second.items;
    std::string text;
    text.reserve(pixels.size());
    for (uint32_t px : pixels) {
        auto it = lookup.find(px);
        if (it != lookup.end() && it->second < textItems.size()) {
            text += textItems[it->second].string;
        }
    }
    return text;
}

} // namespace

const GameInfo *knownGames(int &count)
{
    count = static_cast<int>(SDL_arraysize(kGames));
    return kGames;
}

bool Importer::run(const std::string &swfPath, const std::string &dataDir, const ProgressFn &progress,
                   const std::string &expectedGame)
{
    auto report = [&](float f, const std::string &step) {
        if (progress) {
            progress(Progress{f, step});
        }
    };
    report(0.0f, "Reading file");
    size_t size = 0;
    void *raw = SDL_LoadFile(swfPath.c_str(), &size);
    if (!raw) {
        error_ = std::string("cannot read the file: ") + SDL_GetError();
        return false;
    }
    const std::vector<uint8_t> file(static_cast<uint8_t *>(raw), static_cast<uint8_t *>(raw) + size);
    SDL_free(raw);

    report(0.05f, "Identifying the game");
    const std::string hash = sha256Hex(file.data(), file.size());
    Swf swf;
    if (!swf.load(file, error_)) {
        return false;
    }
    const GameInfo *game = nullptr;
    for (const GameInfo &g : kGames) {
        if (hash.compare(0, 16, g.sha256Prefix) == 0) {
            game = &g;
        }
    }
    const Rect fs = swf.frameSize();
    const int width = (fs.xmax - fs.xmin) / 20, height = (fs.ymax - fs.ymin) / 20;
    if (!game && !expectedGame.empty()) {
        // Another copy of the game: accept it when its stage matches the game the user picked.
        for (const GameInfo &g : kGames) {
            const bool pl1Shape = width == 550 && height == 400;
            const bool pl23Shape = width == 700 && height == 416;
            if (expectedGame == g.id && ((expectedGame == "pl1" && pl1Shape) || (expectedGame != "pl1" && pl23Shape))) {
                game = &g;
                SDL_Log("Import: unknown copy (sha256 %s), accepted as %s by its stage size", hash.c_str(), g.id);
            }
        }
    }
    if (!game) {
        error_ = "this file is not one of the supported Papa Louie games";
        return false;
    }
    if (!expectedGame.empty() && expectedGame != game->id) {
        error_ = std::string("this file is ") + game->title;
        return false;
    }
    gameId_ = game->id;
    const std::string out = dataDir + "/" + game->id;
    if (!SDL_CreateDirectory(out.c_str())) {
        error_ = std::string("cannot create ") + out + ": " + SDL_GetError();
        return false;
    }

    report(0.10f, "Reading characters");
    const SwfIndex index = indexSwf(swf);
    if (cancel) {
        error_ = "cancelled";
        return false;
    }
    report(0.20f, "Converting timelines");
    const std::vector<uint8_t> symbols = buildSymbols(swf, index);
    if (!writeFile(out + "/symbols.bin", symbols.data(), symbols.size())) {
        error_ = std::string("cannot write symbols.bin: ") + SDL_GetError();
        return false;
    }

    report(0.40f, "Decoding levels");
    std::string note;
    const std::string levels = decodeLevels(index, note);
    if (!levels.empty()) {
        writeFile(out + "/levels.xml", levels.data(), levels.size());
    } else {
        SDL_Log("Import: no levels.xml (%s)", note.c_str());
    }

    report(0.42f, "Reading game tables");
    {
        // Constant tables of papaGame.data.DataManager (tile collision/slope lists, ...), kept out of the repo
        // like the level tables: datamanager.json, { "field": value, ... }.
        std::map<std::string, AbcValue> fields;
        for (const Tag &tag : index.abc) {
            size_t p = 0;
            if (tag.code == 82) {
                p = 4;
                cstring(tag.body, tag.size, p);
            }
            Abc abc;
            if (abc.parse(tag.body + p, tag.size - p)) {
                fields = abc.instanceFieldConstants("papaGame.data", "DataManager");
                if (!fields.empty()) {
                    break;
                }
            }
        }
        std::string json = "{\n";
        bool first = true;
        for (const auto &[name, value] : fields) {
            if (value.type == AbcValue::Type::Unknown) {
                continue;
            }
            json += first ? "" : ",\n";
            first = false;
            json += "\"" + name + "\": ";
            appendJson(json, value);
        }
        json += "\n}\n";
        writeFile(out + "/datamanager.json", json.data(), json.size());
    }
    report(0.43f, "Converting frame scripts");
    {
        // Timeline scripts (stop, gotoAndPlay, part poses) become data: <class> <frame> <op> [target] [number].
        std::string text;
        size_t skipped = 0;
        for (const Tag &tag : index.abc) {
            size_t p = 0;
            if (tag.code == 82) {
                p = 4;
                cstring(tag.body, tag.size, p);
            }
            Abc abc;
            if (!abc.parse(tag.body + p, tag.size - p)) {
                continue;
            }
            for (const auto &[cls, ops] : abc.frameScripts(skipped)) {
                for (const Abc::FrameOp &op : ops) {
                    char line[256];
                    SDL_snprintf(line, sizeof(line), "%s %u %s %s %g\n", cls.c_str(), op.frame, op.op.c_str(),
                                 op.target.empty() ? "-" : op.target.c_str(), op.number);
                    text += line;
                }
            }
        }
        writeFile(out + "/framescripts.txt", text.data(), text.size());
        SDL_Log("Import: frame scripts converted (%zu statements skipped)", skipped);
    }

    // Vector art at 2x: sharp at the usual window and phone sizes (the stage is 700x416).
    constexpr int kScale = 2;
    Rasterizer raster(swf, index);
    const bool shapesOk = raster.exportShapes(out, kScale, [&](size_t done, size_t total) {
        if (done % 50 == 0 || done == total) {
            report(0.45f + 0.40f * static_cast<float>(done) / static_cast<float>(total),
                   "Drawing shapes " + std::to_string(done) + "/" + std::to_string(total));
        }
        return !cancel;
    });
    if (!shapesOk) {
        error_ = cancel ? "cancelled" : "could not write the shapes";
        return false;
    }
    const std::vector<std::string> sheets = sheetList(game->id);
    if (!sheets.empty()) {
        const bool sheetsOk = raster.exportSheets(out, sheets, kScale, [&](size_t done, size_t total) {
            report(0.85f + 0.14f * static_cast<float>(done) / static_cast<float>(total),
                   "Drawing sheets " + std::to_string(done) + "/" + std::to_string(total));
            return !cancel;
        });
        if (!sheetsOk) {
            error_ = cancel ? "cancelled" : "could not write the sheets";
            return false;
        }
    }

    char manifest[512];
    std::snprintf(manifest, sizeof(manifest),
                  "{\n  \"game\": \"%s\",\n  \"sha256\": \"%s\",\n  \"stage\": {\"width\": %d, \"height\": %d, \"fps\": %g},\n"
                  "  \"importer\": 1\n}\n",
                  game->id, hash.c_str(), width, height, swf.frameRate());
    writeFile(out + "/manifest.json", manifest, SDL_strlen(manifest));
    report(1.0f, "Done");
    return true;
}

} // namespace swfimport
