#pragma once
// The port's translation tables (lang/<code>.json in the repository, compiled in): key -> text for the port's
// own interface (the menu bars that replace the Flipline API's, options). The game's own texts stay in the SWF.
#include <map>
#include <string>
#include <string_view>

namespace engine {

class Lang {
public:
    // "en", "es"...; an empty or unknown code picks the system's preferred language, then English.
    void select(std::string_view code);
    const std::string &code() const { return code_; }
    // The text for `key` in the selected language, else in English, else the key itself.
    const std::string &get(std::string_view key) const;

private:
    std::string code_ = "en";
    std::map<std::string, std::string, std::less<>> table_, english_;
};

} // namespace engine
