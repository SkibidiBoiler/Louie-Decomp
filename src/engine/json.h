#pragma once
// Minimal JSON reader for the port's data files (numbers, strings, arrays, objects, true/false/null).
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

struct JsonValue {
    enum class Type { Null, Bool, Number, String, Array, Object } type = Type::Null;
    bool boolean = false;
    double number = 0;
    std::string string;
    std::vector<JsonValue> items;
    std::map<std::string, JsonValue> fields;

    const JsonValue *get(std::string_view key) const
    {
        auto it = fields.find(std::string(key));
        return it == fields.end() ? nullptr : &it->second;
    }
};

// Parses a document; false on malformed input.
bool parseJson(std::string_view text, JsonValue &out);

} // namespace engine
