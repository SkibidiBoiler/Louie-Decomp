#pragma once
// Minimal AVM2 (ABC) reader for the importer: the constant pool, classes and method bodies, enough to read the
// constant values a class assigns to its fields (e.g. papaGame.data.LevelData's decode tables). It reads
// constants only; it never runs code.
#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace swfimport {

// A constant the scanner can follow: a number, a string, or an array of them.
struct AbcValue {
    enum class Type { Unknown, Number, String, Array } type = Type::Unknown;
    double number = 0;
    std::string string;
    std::vector<AbcValue> items;
};

class Abc {
public:
    bool parse(const uint8_t *data, size_t size);

    // Field values set by the instance initialiser (and slot defaults) of class `ns`.`name`.
    // Empty when the class is not in this ABC block.
    std::map<std::string, AbcValue> instanceFieldConstants(const std::string &ns, const std::string &name) const;

    // One recognised statement of a timeline frame script.
    struct FrameOp {
        uint16_t frame = 0;      // 0-based frame the script belongs to
        std::string op;          // "stop", "play", "gotoAndPlay", "gotoAndStop", "child"
        std::string target;      // label (goto*) or container instance name (child)
        double number = 0;       // frame number (1-based) for goto*/child, when not a label
    };
    // Frame scripts of every class in this block that calls addFrameScript: class name -> statements.
    // Statements the converter does not recognise are counted in `skipped`.
    std::map<std::string, std::vector<FrameOp>> frameScripts(size_t &skipped) const;

    struct Trait {
        uint32_t name = 0;
        uint8_t kind = 0;
        uint32_t vindex = 0; // slot default value index, or the method index of a method trait
        uint8_t vkind = 0;
    };

private:
    struct Multiname {
        uint8_t kind = 0;
        uint32_t ns = 0, name = 0;
    };
    struct Instance {
        uint32_t name = 0;
        uint32_t iinit = 0;
        std::vector<Trait> traits;
    };
    struct Body {
        std::vector<uint8_t> code;
    };

    std::string multinameName(uint32_t index) const;
    std::string multinameNs(uint32_t index) const;
    AbcValue constant(uint32_t index, uint8_t kind) const;

    std::vector<int32_t> ints_;
    std::vector<uint32_t> uints_;
    std::vector<double> doubles_;
    std::vector<std::string> strings_;
    std::vector<std::pair<uint8_t, uint32_t>> namespaces_;
    std::vector<Multiname> multinames_;
    std::vector<Instance> instances_;
    std::map<uint32_t, Body> bodies_; // method index -> body
};

} // namespace swfimport
