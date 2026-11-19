#include "engine/shared_object.h"

#include <SDL3/SDL.h>

namespace engine {

std::string SharedObject::path(const std::string &name)
{
    char *pref = SDL_GetPrefPath("SkibidiBoiler", "PapaLouieRecomp");
    if (!pref) {
        return {};
    }
    std::string dir = std::string(pref) + "saves";
    SDL_free(pref);
    SDL_CreateDirectory(dir.c_str());
    return dir + "/" + name + ".json";
}

SharedObject SharedObject::getLocal(const std::string &name)
{
    SharedObject so;
    so.name = name;
    so.data.type = JsonValue::Type::Object;
    const std::string file = path(name);
    size_t size = 0;
    char *raw = file.empty() ? nullptr : static_cast<char *>(SDL_LoadFile(file.c_str(), &size));
    if (raw) {
        JsonValue json;
        if (parseJson(std::string_view(raw, size), json) && json.type == JsonValue::Type::Object) {
            so.data = std::move(json);
        } else {
            SDL_Log("Save %s is damaged; starting it empty", file.c_str());
        }
        SDL_free(raw);
    }
    return so;
}

bool SharedObject::flush() const
{
    const std::string file = path(name);
    if (file.empty()) {
        return false;
    }
    // Written to a temporary file first, so a crash mid-write keeps the old save.
    const std::string text = writeJson(data);
    const std::string temp = file + ".tmp";
    if (!SDL_SaveFile(temp.c_str(), text.data(), text.size())) {
        SDL_Log("Cannot save %s: %s", file.c_str(), SDL_GetError());
        return false;
    }
    return SDL_RenamePath(temp.c_str(), file.c_str());
}

void SharedObject::clear()
{
    data = JsonValue{};
    data.type = JsonValue::Type::Object;
    const std::string file = path(name);
    if (!file.empty()) {
        SDL_RemovePath(file.c_str());
    }
}

bool SharedObject::truthy(const std::string &field) const
{
    const JsonValue *v = get(field);
    if (!v) {
        return false;
    }
    switch (v->type) {
    case JsonValue::Type::Null: return false;
    case JsonValue::Type::Bool: return v->boolean;
    case JsonValue::Type::Number: return v->number != 0 && v->number == v->number;
    case JsonValue::Type::String: return !v->string.empty();
    default: return true;
    }
}

JsonValue jsonNumber(double v)
{
    JsonValue j;
    j.type = JsonValue::Type::Number;
    j.number = v;
    return j;
}

JsonValue jsonString(const std::string &s)
{
    JsonValue j;
    j.type = JsonValue::Type::String;
    j.string = s;
    return j;
}

JsonValue jsonBool(bool b)
{
    JsonValue j;
    j.type = JsonValue::Type::Bool;
    j.boolean = b;
    return j;
}

JsonValue jsonArray(const std::vector<double> &list)
{
    JsonValue j;
    j.type = JsonValue::Type::Array;
    for (double v : list) {
        j.items.push_back(jsonNumber(v));
    }
    return j;
}

JsonValue jsonArray2(const std::vector<std::vector<double>> &list)
{
    JsonValue j;
    j.type = JsonValue::Type::Array;
    for (const auto &row : list) {
        j.items.push_back(jsonArray(row));
    }
    return j;
}

double jsonToNumber(const JsonValue *v)
{
    if (!v) {
        return 0;
    }
    if (v->type == JsonValue::Type::Number) {
        return v->number;
    }
    if (v->type == JsonValue::Type::Bool) {
        return v->boolean ? 1 : 0;
    }
    return 0;
}

std::vector<double> jsonToArray(const JsonValue *v)
{
    std::vector<double> out;
    if (v && v->type == JsonValue::Type::Array) {
        for (const JsonValue &item : v->items) {
            out.push_back(jsonToNumber(&item));
        }
    }
    return out;
}

std::vector<std::vector<double>> jsonToArray2(const JsonValue *v)
{
    std::vector<std::vector<double>> out;
    if (v && v->type == JsonValue::Type::Array) {
        for (const JsonValue &item : v->items) {
            out.push_back(jsonToArray(&item));
        }
    }
    return out;
}

} // namespace engine
