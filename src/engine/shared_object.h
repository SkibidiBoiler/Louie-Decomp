#pragma once
// flash.net.SharedObject for the save slots: `data` is a JSON object kept in <pref path>/saves/<name>.json.
// Flash stored the slots in the browser's Flash cookies; the port's files are not compatible with them.
#include <string>

#include "engine/json.h"

namespace engine {

class SharedObject {
public:
    // SharedObject.getLocal(name): the stored data, or an empty object.
    static SharedObject getLocal(const std::string &name);

    // SharedObject.flush(): writes `data`; false when the file cannot be written.
    bool flush() const;
    // SharedObject.clear(): forgets the data and deletes the file.
    void clear();

    std::string name;
    JsonValue data;

    // Helpers for `so.data.<field>`, with ActionScript's truthiness.
    const JsonValue *get(const std::string &field) const { return data.get(field); }
    bool truthy(const std::string &field) const;
    void set(const std::string &field, JsonValue value) { data.fields[field] = std::move(value); }

private:
    static std::string path(const std::string &name);
};

// JSON values from numbers, strings, booleans and (nested) arrays of numbers.
JsonValue jsonNumber(double v);
JsonValue jsonString(const std::string &s);
JsonValue jsonBool(bool b);
JsonValue jsonArray(const std::vector<double> &list);
JsonValue jsonArray2(const std::vector<std::vector<double>> &list);
// Reads them back (missing or other types give 0 / empty).
double jsonToNumber(const JsonValue *v);
std::vector<double> jsonToArray(const JsonValue *v);
std::vector<std::vector<double>> jsonToArray2(const JsonValue *v);

} // namespace engine
