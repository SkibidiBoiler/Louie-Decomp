#include "engine/xml.h"

#include <SDL3/SDL.h>

namespace engine {

namespace {

void appendDecoded(std::string &out, std::string_view raw)
{
    for (size_t i = 0; i < raw.size(); ++i) {
        if (raw[i] != '&') {
            out += raw[i];
            continue;
        }
        const size_t end = raw.find(';', i);
        if (end == std::string_view::npos) {
            out += raw[i];
            continue;
        }
        const std::string_view entity = raw.substr(i + 1, end - i - 1);
        if (entity == "lt") {
            out += '<';
        } else if (entity == "gt") {
            out += '>';
        } else if (entity == "amp") {
            out += '&';
        } else if (entity == "quot") {
            out += '"';
        } else if (entity == "apos") {
            out += '\'';
        } else if (!entity.empty() && entity[0] == '#') {
            const bool hex = entity.size() > 1 && (entity[1] == 'x' || entity[1] == 'X');
            const unsigned long code = std::strtoul(std::string(entity.substr(hex ? 2 : 1)).c_str(), nullptr, hex ? 16 : 10);
            if (code < 0x80) {
                out += static_cast<char>(code);
            } // non-ASCII references do not occur in the game data
        } else {
            out.append(raw.substr(i, end - i + 1));
        }
        i = end;
    }
}

} // namespace

const XmlNode *XmlNode::child(std::string_view name) const
{
    for (const auto &c : children) {
        if (c->tag == name) {
            return c.get();
        }
    }
    return nullptr;
}

const std::string &XmlNode::childText(std::string_view name) const
{
    static const std::string empty;
    const XmlNode *c = child(name);
    return c ? c->text : empty;
}

std::vector<const XmlNode *> XmlNode::childrenNamed(std::string_view name) const
{
    std::vector<const XmlNode *> out;
    for (const auto &c : children) {
        if (c->tag == name) {
            out.push_back(c.get());
        }
    }
    return out;
}

std::unique_ptr<XmlNode> parseXml(std::string_view text)
{
    std::vector<XmlNode *> stack;
    std::unique_ptr<XmlNode> root;
    size_t i = 0;
    while (i < text.size()) {
        if (text[i] != '<') {
            const size_t next = text.find('<', i);
            const std::string_view chunk = text.substr(i, next == std::string_view::npos ? std::string_view::npos : next - i);
            if (!stack.empty()) {
                appendDecoded(stack.back()->text, chunk);
            }
            i = next == std::string_view::npos ? text.size() : next;
            continue;
        }
        const size_t close = text.find('>', i);
        if (close == std::string_view::npos) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "XML: unterminated tag at %zu", i);
            return nullptr;
        }
        std::string_view tag = text.substr(i + 1, close - i - 1);
        i = close + 1;
        if (tag.empty() || tag[0] == '?' || tag[0] == '!') {
            continue; // declaration, comment or doctype
        }
        if (tag[0] == '/') {
            if (stack.empty()) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "XML: unexpected closing tag");
                return nullptr;
            }
            stack.pop_back();
            continue;
        }
        const bool selfClosing = tag.back() == '/';
        if (selfClosing) {
            tag.remove_suffix(1);
        }
        const size_t space = tag.find_first_of(" \t\r\n");
        auto node = std::make_unique<XmlNode>();
        node->tag = std::string(tag.substr(0, space));
        XmlNode *raw = node.get();
        if (stack.empty()) {
            if (root) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "XML: more than one root element");
                return nullptr;
            }
            root = std::move(node);
        } else {
            stack.back()->children.push_back(std::move(node));
        }
        if (!selfClosing) {
            stack.push_back(raw);
        }
    }
    if (!stack.empty() || !root) {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "XML: unbalanced document");
        return nullptr;
    }
    return root;
}

} // namespace engine
