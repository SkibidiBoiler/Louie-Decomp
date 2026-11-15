#include "import/abc.h"

#include <cstring>

namespace swfimport {

namespace {

class Reader {
public:
    Reader(const uint8_t *d, size_t n) : d_(d), n_(n) {}
    bool ok() const { return ok_; }
    size_t pos() const { return p_; }
    uint8_t u8()
    {
        if (p_ >= n_) {
            ok_ = false;
            return 0;
        }
        return d_[p_++];
    }
    uint16_t u16() { const uint8_t a = u8(); return static_cast<uint16_t>(a | (u8() << 8)); }
    int32_t s24()
    {
        uint32_t v = u8();
        v |= static_cast<uint32_t>(u8()) << 8;
        v |= static_cast<uint32_t>(u8()) << 16;
        return (v & 0x800000u) ? static_cast<int32_t>(v | 0xFF000000u) : static_cast<int32_t>(v);
    }
    uint32_t u30()
    {
        uint32_t v = 0;
        for (int i = 0; i < 5; ++i) {
            const uint8_t b = u8();
            v |= static_cast<uint32_t>(b & 0x7F) << (7 * i);
            if (!(b & 0x80)) {
                break;
            }
        }
        return v;
    }
    double d64()
    {
        uint8_t b[8];
        for (uint8_t &x : b) {
            x = u8();
        }
        double v;
        std::memcpy(&v, b, 8);
        return v;
    }
    std::string str(uint32_t len)
    {
        if (p_ + len > n_) {
            ok_ = false;
            return {};
        }
        std::string s(reinterpret_cast<const char *>(d_ + p_), len);
        p_ += len;
        return s;
    }
    void seek(size_t p) { p_ = p <= n_ ? p : (ok_ = false, n_); }
    void skip(size_t len) { p_ = p_ + len > n_ ? (ok_ = false, n_) : p_ + len; }

private:
    const uint8_t *d_;
    size_t n_;
    size_t p_ = 0;
    bool ok_ = true;
};

void readTraits(Reader &r, std::vector<Abc::Trait> *out);

} // namespace

bool Abc::parse(const uint8_t *data, size_t size)
{
    Reader r(data, size);
    r.u16(); // minor
    r.u16(); // major
    uint32_t n = r.u30();
    ints_.assign(1, 0);
    for (uint32_t i = 1; i < n; ++i) {
        ints_.push_back(static_cast<int32_t>(r.u30()));
    }
    n = r.u30();
    uints_.assign(1, 0);
    for (uint32_t i = 1; i < n; ++i) {
        uints_.push_back(r.u30());
    }
    n = r.u30();
    doubles_.assign(1, 0.0);
    for (uint32_t i = 1; i < n; ++i) {
        doubles_.push_back(r.d64());
    }
    n = r.u30();
    strings_.assign(1, std::string());
    for (uint32_t i = 1; i < n; ++i) {
        strings_.push_back(r.str(r.u30()));
    }
    n = r.u30();
    namespaces_.assign(1, {0, 0});
    for (uint32_t i = 1; i < n; ++i) {
        const uint8_t kind = r.u8();
        namespaces_.emplace_back(kind, r.u30());
    }
    n = r.u30();
    for (uint32_t i = 1; i < n; ++i) { // namespace sets
        const uint32_t count = r.u30();
        for (uint32_t j = 0; j < count; ++j) {
            r.u30();
        }
    }
    n = r.u30();
    multinames_.assign(1, Multiname{});
    for (uint32_t i = 1; i < n && r.ok(); ++i) {
        Multiname m;
        m.kind = r.u8();
        switch (m.kind) {
        case 0x07: case 0x0D: m.ns = r.u30(); m.name = r.u30(); break;  // QName(A)
        case 0x0F: case 0x10: m.name = r.u30(); break;                  // RTQName(A)
        case 0x11: case 0x12: break;                                    // RTQNameL(A)
        case 0x09: case 0x0E: m.name = r.u30(); r.u30(); break;         // Multiname(A)
        case 0x1B: case 0x1C: r.u30(); break;                           // MultinameL(A)
        case 0x1D: {                                                    // TypeName
            m.name = r.u30();
            const uint32_t count = r.u30();
            for (uint32_t j = 0; j < count; ++j) {
                r.u30();
            }
            break;
        }
        default: return false;
        }
        multinames_.push_back(m);
    }
    n = r.u30(); // methods
    for (uint32_t i = 0; i < n && r.ok(); ++i) {
        const uint32_t params = r.u30();
        r.u30(); // return type
        for (uint32_t j = 0; j < params; ++j) {
            r.u30();
        }
        r.u30(); // name
        const uint8_t flags = r.u8();
        if (flags & 0x08) {
            const uint32_t options = r.u30();
            for (uint32_t j = 0; j < options; ++j) {
                r.u30();
                r.u8();
            }
        }
        if (flags & 0x80) {
            for (uint32_t j = 0; j < params; ++j) {
                r.u30();
            }
        }
    }
    n = r.u30(); // metadata
    for (uint32_t i = 0; i < n && r.ok(); ++i) {
        r.u30();
        const uint32_t items = r.u30();
        for (uint32_t j = 0; j < items; ++j) {
            r.u30();
            r.u30();
        }
    }
    const uint32_t classCount = r.u30();
    instances_.resize(classCount);
    for (uint32_t i = 0; i < classCount && r.ok(); ++i) {
        Instance &inst = instances_[i];
        inst.name = r.u30();
        r.u30(); // super
        const uint8_t flags = r.u8();
        if (flags & 0x08) {
            r.u30(); // protected namespace
        }
        const uint32_t interfaces = r.u30();
        for (uint32_t j = 0; j < interfaces; ++j) {
            r.u30();
        }
        inst.iinit = r.u30();
        readTraits(r, &inst.traits);
    }
    for (uint32_t i = 0; i < classCount && r.ok(); ++i) {
        r.u30(); // cinit
        readTraits(r, nullptr);
    }
    n = r.u30(); // scripts
    for (uint32_t i = 0; i < n && r.ok(); ++i) {
        r.u30();
        readTraits(r, nullptr);
    }
    n = r.u30(); // method bodies
    for (uint32_t i = 0; i < n && r.ok(); ++i) {
        const uint32_t method = r.u30();
        r.u30();
        r.u30();
        r.u30();
        r.u30();
        const uint32_t len = r.u30();
        const std::string code = r.str(len);
        bodies_[method].code.assign(code.begin(), code.end());
        const uint32_t exceptions = r.u30();
        for (uint32_t j = 0; j < exceptions; ++j) {
            for (int k = 0; k < 5; ++k) {
                r.u30();
            }
        }
        readTraits(r, nullptr);
    }
    return r.ok();
}

namespace {

void readTraits(Reader &r, std::vector<Abc::Trait> *out)
{
    const uint32_t count = r.u30();
    for (uint32_t i = 0; i < count && r.ok(); ++i) {
        Abc::Trait t;
        t.name = r.u30();
        t.kind = r.u8();
        switch (t.kind & 0x0F) {
        case 0: case 6: // slot / const
            r.u30();    // slot id
            r.u30();    // type
            t.vindex = r.u30();
            if (t.vindex) {
                t.vkind = r.u8();
            }
            break;
        case 1: case 2: case 3: case 4: case 5:
            r.u30();
            r.u30();
            break;
        default:
            break;
        }
        if ((t.kind >> 4) & 0x04) {
            const uint32_t meta = r.u30();
            for (uint32_t j = 0; j < meta; ++j) {
                r.u30();
            }
        }
        if (out) {
            out->push_back(t);
        }
    }
}

// Operand layout of every AVM2 opcode, so the scanner can step over instructions it does not interpret.
enum Op : uint8_t { None, U8, U30, U30x2, S24, Switch, Debug };

Op operandsOf(uint8_t op)
{
    switch (op) {
    case 0x24: return U8; // pushbyte
    case 0x08: case 0x20: case 0x21: case 0x22: case 0x23: case 0x25: case 0x26: case 0x27: case 0x28:
    case 0x2C: case 0x2D: case 0x2E: case 0x2F: case 0x31: case 0x40: case 0x41: case 0x42: case 0x49:
    case 0x53: case 0x55: case 0x56: case 0x57: case 0x58: case 0x59: case 0x5A: case 0x5D: case 0x5E:
    case 0x5F: case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x68:
    case 0x6A: case 0x6C: case 0x6D: case 0x6E: case 0x6F: case 0x80: case 0x86: case 0x92: case 0x94:
    case 0xB2: case 0xC2: case 0xC3: case 0xF0: case 0xF1: case 0x04: case 0x05: case 0x06:
        return U30;
    case 0x43: case 0x44: case 0x45: case 0x46: case 0x4A: case 0x4C: case 0x4E: case 0x4F: case 0x32:
        return U30x2;
    case 0x0C: case 0x0D: case 0x0E: case 0x0F: case 0x10: case 0x11: case 0x12: case 0x13: case 0x14:
    case 0x15: case 0x16: case 0x17: case 0x18: case 0x19: case 0x1A:
        return S24;
    case 0x1B: return Switch;
    case 0xEF: return Debug;
    default: return None;
    }
}

} // namespace

std::string Abc::multinameName(uint32_t index) const
{
    if (index == 0 || index >= multinames_.size()) {
        return {};
    }
    const uint32_t name = multinames_[index].name;
    return name < strings_.size() ? strings_[name] : std::string();
}

std::string Abc::multinameNs(uint32_t index) const
{
    if (index == 0 || index >= multinames_.size()) {
        return {};
    }
    const uint32_t ns = multinames_[index].ns;
    if (ns == 0 || ns >= namespaces_.size()) {
        return {};
    }
    const uint32_t name = namespaces_[ns].second;
    return name < strings_.size() ? strings_[name] : std::string();
}

AbcValue Abc::constant(uint32_t index, uint8_t kind) const
{
    AbcValue v;
    switch (kind) {
    case 0x01: if (index < strings_.size()) { v.type = AbcValue::Type::String; v.string = strings_[index]; } break;
    case 0x03: if (index < ints_.size()) { v.type = AbcValue::Type::Number; v.number = ints_[index]; } break;
    case 0x04: if (index < uints_.size()) { v.type = AbcValue::Type::Number; v.number = uints_[index]; } break;
    case 0x06: if (index < doubles_.size()) { v.type = AbcValue::Type::Number; v.number = doubles_[index]; } break;
    default: break;
    }
    return v;
}

std::map<std::string, AbcValue> Abc::instanceFieldConstants(const std::string &ns, const std::string &name) const
{
    std::map<std::string, AbcValue> fields;
    for (const Instance &inst : instances_) {
        if (multinameName(inst.name) != name || multinameNs(inst.name) != ns) {
            continue;
        }
        for (const Trait &t : inst.traits) {
            if ((t.kind & 0x0F) == 0 || (t.kind & 0x0F) == 6) {
                if (t.vindex) {
                    fields[multinameName(t.name)] = constant(t.vindex, t.vkind);
                }
            }
        }
        auto body = bodies_.find(inst.iinit);
        if (body == bodies_.end()) {
            return fields;
        }
        // Follow constant pushes, array literals and field stores in the initialiser, in code order.
        const auto &code = body->second.code;
        Reader r(code.data(), code.size());
        std::vector<AbcValue> stack;
        auto push = [&](AbcValue v) { stack.push_back(std::move(v)); };
        auto pop = [&]() {
            if (stack.empty()) {
                return AbcValue{};
            }
            AbcValue v = std::move(stack.back());
            stack.pop_back();
            return v;
        };
        // Unconditional jumps are followed, and conditional ones are decided when both operands are known
        // constants (the obfuscator's opaque predicates); otherwise the scan falls through.
        size_t steps = 0;
        while (r.ok() && r.pos() < code.size() && ++steps < 1000000) {
            const uint8_t op = r.u8();
            AbcValue v;
            switch (op) {
            case 0x24: v.type = AbcValue::Type::Number; v.number = static_cast<int8_t>(r.u8()); push(v); continue;
            case 0x25: v.type = AbcValue::Type::Number; v.number = static_cast<int16_t>(r.u30()); push(v); continue;
            case 0x2D: push(constant(r.u30(), 0x03)); continue;
            case 0x2E: push(constant(r.u30(), 0x04)); continue;
            case 0x2F: push(constant(r.u30(), 0x06)); continue;
            case 0x2C: push(constant(r.u30(), 0x01)); continue;
            case 0x56: { // newarray
                const uint32_t count = r.u30();
                v.type = AbcValue::Type::Array;
                v.items.resize(count);
                for (uint32_t i = count; i-- > 0;) {
                    v.items[i] = pop();
                }
                push(std::move(v));
                continue;
            }
            case 0x61: case 0x68: { // setproperty / initproperty
                const uint32_t mn = r.u30();
                AbcValue value = pop();
                pop(); // object
                if (value.type != AbcValue::Type::Unknown) {
                    fields[multinameName(mn)] = std::move(value);
                }
                continue;
            }
            case 0x10: { // jump
                const int32_t off = r.s24();
                r.seek(static_cast<size_t>(static_cast<int64_t>(r.pos()) + off));
                continue;
            }
            case 0x11: case 0x12: { // iftrue / iffalse
                const int32_t off = r.s24();
                const AbcValue a = pop();
                if (a.type == AbcValue::Type::Number) {
                    const bool truthy = a.number != 0 && a.number == a.number;
                    if (truthy == (op == 0x11)) {
                        r.seek(static_cast<size_t>(static_cast<int64_t>(r.pos()) + off));
                    }
                }
                continue;
            }
            case 0x0C: case 0x0D: case 0x0E: case 0x0F: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
            case 0x18: case 0x19: case 0x1A: { // compare-and-branch
                const int32_t off = r.s24();
                const AbcValue b = pop();
                const AbcValue a = pop();
                if (a.type == AbcValue::Type::Number && b.type == AbcValue::Type::Number) {
                    const double x = a.number, y = b.number;
                    bool take = false;
                    switch (op) {
                    case 0x0C: take = !(x < y); break;  // ifnlt
                    case 0x0D: take = !(x <= y); break; // ifnle
                    case 0x0E: take = !(x > y); break;  // ifngt
                    case 0x0F: take = !(x >= y); break; // ifnge
                    case 0x13: case 0x19: take = x == y; break; // ifeq / ifstricteq
                    case 0x14: case 0x1A: take = x != y; break; // ifne / ifstrictne
                    case 0x15: take = x < y; break;     // iflt
                    case 0x16: take = x <= y; break;    // ifle
                    case 0x17: take = x > y; break;     // ifgt
                    case 0x18: take = x >= y; break;    // ifge
                    default: break;
                    }
                    if (take) {
                        r.seek(static_cast<size_t>(static_cast<int64_t>(r.pos()) + off));
                    }
                }
                continue;
            }
            case 0x26: v.type = AbcValue::Type::Number; v.number = 1; push(v); continue; // pushtrue
            case 0x27: v.type = AbcValue::Type::Number; v.number = 0; push(v); continue; // pushfalse
            case 0x29: pop(); continue;                         // pop
            case 0x2A: if (!stack.empty()) push(stack.back()); continue; // dup
            case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0x20: case 0x28:
                push(AbcValue{}); // getlocalN, pushnull, pushnan
                continue;
            case 0x5D: case 0x5E: r.u30(); push(AbcValue{}); continue; // findpropstrict / findproperty
            default: break;
            }
            switch (operandsOf(op)) {
            case U8: r.u8(); break;
            case U30: r.u30(); break;
            case U30x2: r.u30(); r.u30(); break;
            case S24: r.s24(); break;
            case Switch: {
                r.s24();
                const uint32_t cases = r.u30();
                for (uint32_t i = 0; i <= cases; ++i) {
                    r.s24();
                }
                break;
            }
            case Debug: r.u8(); r.u30(); r.u8(); r.u30(); break;
            case None: break;
            }
        }
        return fields;
    }
    return fields;
}

} // namespace swfimport
