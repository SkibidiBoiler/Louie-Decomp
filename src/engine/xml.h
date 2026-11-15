#pragma once
// Minimal XML reader for the games' data files: elements and text only (no attributes, namespaces or DTDs).
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace engine {

struct XmlNode {
    std::string tag;
    std::string text; // concatenated character data, entities decoded
    std::vector<std::unique_ptr<XmlNode>> children;

    // First child with this tag, or nullptr.
    const XmlNode *child(std::string_view name) const;
    // Text of the first child with this tag; "" when missing (like AS3's String(xml.missing)).
    const std::string &childText(std::string_view name) const;
    // Every child with this tag.
    std::vector<const XmlNode *> childrenNamed(std::string_view name) const;
};

// Parses a document; returns nullptr (and logs) on malformed input.
std::unique_ptr<XmlNode> parseXml(std::string_view text);

} // namespace engine
