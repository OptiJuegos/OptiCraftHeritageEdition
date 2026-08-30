#pragma once

#include <string>
#include <vector>
#include <map>

class J_JsonNode;

// Type-erased value that flows through the selector chain.
// Possible kinds: a JSON node pointer, a string value, an array list, or an object field map.
struct SelectorValue
{
    enum Kind { NODE, STR, ARR, OBJ };
    Kind kind;

    J_JsonNode*                              node;
    std::string                              str;
    std::vector<J_JsonNode*>                 arr;
    std::map<std::string, J_JsonNode*>       obj;

    explicit SelectorValue(J_JsonNode* n)
        : kind(NODE), node(n) {}

    explicit SelectorValue(const std::string& s)
        : kind(STR), node(nullptr), str(s) {}

    explicit SelectorValue(const std::vector<J_JsonNode*>& a)
        : kind(ARR), node(nullptr), arr(a) {}

    explicit SelectorValue(const std::map<std::string, J_JsonNode*>& o)
        : kind(OBJ), node(nullptr), obj(o) {}

private:
    SelectorValue() : kind(NODE), node(nullptr) {}
};

// net.minecraft.src.J_Functor
class J_Functor
{
public:
    virtual ~J_Functor() {}
    virtual bool          matches(const SelectorValue& val) = 0;  // func_27058_a
    virtual SelectorValue getValue(const SelectorValue& val) = 0; // func_27059_b
    virtual std::string   getName() = 0;                           // func_27060_a
    virtual std::string   toString() { return getName(); }
};
