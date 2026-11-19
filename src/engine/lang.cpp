#include "engine/lang.h"

#include <SDL3/SDL.h>

#include "engine/json.h"
#include "lang_tables.h"

namespace engine {

namespace {

bool loadTable(std::string_view code, std::map<std::string, std::string, std::less<>> &out)
{
    for (const LangTable &t : kLangTables) {
        if (code == t.code) {
            JsonValue json;
            if (!parseJson(t.json, json)) {
                SDL_Log("Lang: lang/%s.json is not valid JSON", t.code);
                return false;
            }
            out.clear();
            for (const auto &[key, value] : json.fields) {
                out[key] = value.string;
            }
            return true;
        }
    }
    return false;
}

} // namespace

void Lang::select(std::string_view code)
{
    loadTable("en", english_);
    std::string wanted(code);
    if (wanted.empty()) {
        int count = 0;
        if (SDL_Locale **locales = SDL_GetPreferredLocales(&count)) {
            for (int i = 0; i < count && wanted.empty(); ++i) {
                std::map<std::string, std::string, std::less<>> probe;
                if (loadTable(locales[i]->language, probe)) {
                    wanted = locales[i]->language;
                }
            }
            SDL_free(locales);
        }
    }
    if (wanted.empty() || !loadTable(wanted, table_)) {
        wanted = "en";
        table_ = english_;
    }
    code_ = wanted;
}

const std::string &Lang::get(std::string_view key) const
{
    if (auto it = table_.find(key); it != table_.end()) {
        return it->second;
    }
    if (auto it = english_.find(key); it != english_.end()) {
        return it->second;
    }
    static std::string missing;
    missing = std::string(key);
    return missing;
}

} // namespace engine
