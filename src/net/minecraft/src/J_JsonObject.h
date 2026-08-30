#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdexcept>

#include "J_JsonRootNode.h"
#include "EnumJsonNodeType.h"

// net.minecraft.src.J_JsonObject
class J_JsonObject : public J_JsonRootNode
{
public:
    explicit J_JsonObject(const std::map<std::string, J_JsonNode*>& fields);

    EnumJsonNodeType                    getType()     override;  // func_27218_a
    std::string                         getText()     override;  // func_27216_b
    std::map<std::string, J_JsonNode*>  getFields()   override;  // func_27214_c
    std::vector<J_JsonNode*>            getElements() override;  // func_27215_d

    bool        operator==(const J_JsonObject& other) const;
    std::string toString() const;

private:
    std::map<std::string, J_JsonNode*> fields;  // field_27222_a
};
