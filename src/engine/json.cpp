#include "engine/json.h"

#include <cstdlib>

namespace engine {

namespace {

class Parser {
public:
    explicit Parser(std::string_view t) : t_(t) {}

    bool value(JsonValue &v, int depth = 0)
    {
        skip();
        if (p_ >= t_.size() || depth > 64) {
            return false;
        }
        const char c = t_[p_];
        if (c == '{') {
            ++p_;
            v.type = JsonValue::Type::Object;
            skip();
            if (peek('}')) {
                ++p_;
                return true;
            }
            while (true) {
                JsonValue key;
                skip();
                if (!str(key)) {
                    return false;
                }
                skip();
                if (!peek(':')) {
                    return false;
                }
                ++p_;
                if (!value(v.fields[key.string], depth + 1)) {
                    return false;
                }
                skip();
                if (peek(',')) {
                    ++p_;
                    continue;
                }
                if (peek('}')) {
                    ++p_;
                    return true;
                }
                return false;
            }
        }
        if (c == '[') {
            ++p_;
            v.type = JsonValue::Type::Array;
            skip();
            if (peek(']')) {
                ++p_;
                return true;
            }
            while (true) {
                v.items.emplace_back();
                if (!value(v.items.back(), depth + 1)) {
                    return false;
                }
                skip();
                if (peek(',')) {
                    ++p_;
                    continue;
                }
                if (peek(']')) {
                    ++p_;
                    return true;
                }
                return false;
            }
        }
        if (c == '"') {
            return str(v);
        }
        if (t_.substr(p_, 4) == "true" || t_.substr(p_, 5) == "false") {
            v.type = JsonValue::Type::Bool;
            v.boolean = t_[p_] == 't';
            p_ += v.boolean ? 4 : 5;
            return true;
        }
        if (t_.substr(p_, 4) == "null") {
            p_ += 4;
            return true;
        }
        const std::string num(t_.substr(p_, std::min<size_t>(64, t_.size() - p_)));
        char *end = nullptr;
        v.number = std::strtod(num.c_str(), &end);
        if (end == num.c_str()) {
            return false;
        }
        v.type = JsonValue::Type::Number;
        p_ += static_cast<size_t>(end - num.c_str());
        return true;
    }

private:
    void skip()
    {
        while (p_ < t_.size() && (t_[p_] == ' ' || t_[p_] == '\n' || t_[p_] == '\r' || t_[p_] == '\t')) {
            ++p_;
        }
    }
    bool peek(char c) const { return p_ < t_.size() && t_[p_] == c; }

    bool str(JsonValue &v)
    {
        if (!peek('"')) {
            return false;
        }
        ++p_;
        v.type = JsonValue::Type::String;
        while (p_ < t_.size() && t_[p_] != '"') {
            char c = t_[p_++];
            if (c == '\\' && p_ < t_.size()) {
                c = t_[p_++];
                if (c == 'n') c = '\n';
                else if (c == 't') c = '\t';
                else if (c == 'r') c = '\r';
                else if (c == 'u' && p_ + 4 <= t_.size()) {
                    const unsigned code = static_cast<unsigned>(std::strtoul(std::string(t_.substr(p_, 4)).c_str(), nullptr, 16));
                    p_ += 4;
                    // UTF-8 encode (the data files only use the basic plane).
                    if (code < 0x80) {
                        v.string += static_cast<char>(code);
                    } else if (code < 0x800) {
                        v.string += static_cast<char>(0xC0 | (code >> 6));
                        v.string += static_cast<char>(0x80 | (code & 0x3F));
                    } else {
                        v.string += static_cast<char>(0xE0 | (code >> 12));
                        v.string += static_cast<char>(0x80 | ((code >> 6) & 0x3F));
                        v.string += static_cast<char>(0x80 | (code & 0x3F));
                    }
                    continue;
                }
            }
            v.string += c;
        }
        if (!peek('"')) {
            return false;
        }
        ++p_;
        return true;
    }

    std::string_view t_;
    size_t p_ = 0;
};

} // namespace

bool parseJson(std::string_view text, JsonValue &out)
{
    out = JsonValue{};
    return Parser(text).value(out);
}

} // namespace engine
