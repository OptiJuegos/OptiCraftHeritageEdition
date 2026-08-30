#pragma once

#include <string>
#include <vector>
#include <map>

#include "EnumJsonNodeType.h"

struct JsonPathElement
{
    enum Type { STR, INT };
    Type        type;
    std::string str;
    int         intVal;

    JsonPathElement(const std::string& s) : type(STR), str(s), intVal(0) {}
    JsonPathElement(int i)                : type(INT), intVal(i) {}
};

class J_JsonStringNode;
class J_JsonNodeSelector;

// net.minecraft.src.J_JsonNode
class J_JsonNode
{
public:
    virtual ~J_JsonNode() {}

    virtual EnumJsonNodeType                    getType()     = 0;  // func_27218_a
    virtual std::string                         getText()     = 0;  // func_27216_b
    virtual std::map<std::string, J_JsonNode*>  getFields()   = 0;  // func_27214_c
    virtual std::vector<J_JsonNode*>            getElements() = 0;  // func_27215_d

    // Navigate to a string value along a mixed path (strings = field names, ints = array indices).
    std::string              getStringAt(const std::vector<JsonPathElement>& path);   // func_27213_a
    // Navigate to an array value along a mixed path.
    std::vector<J_JsonNode*> getArrayAt(const std::vector<JsonPathElement>& path);    // func_27217_b

private:
    void* getValueAt(J_JsonNodeSelector* selector, J_JsonNode* root,
                     const std::vector<JsonPathElement>& path);                        // func_27219_a
};
