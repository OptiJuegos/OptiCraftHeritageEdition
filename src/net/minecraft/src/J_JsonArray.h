#pragma once

#include <vector>
#include <map>
#include <string>
#include <stdexcept>

#include "J_JsonRootNode.h"
#include "EnumJsonNodeType.h"

// net.minecraft.src.J_JsonArray
class J_JsonArray : public J_JsonRootNode
{
public:
    explicit J_JsonArray(const std::vector<J_JsonNode*>& elements);

    EnumJsonNodeType                    getType()     override;  // func_27218_a
    std::string                         getText()     override;  // func_27216_b
    std::map<std::string, J_JsonNode*>  getFields()   override;  // func_27214_c
    std::vector<J_JsonNode*>            getElements() override;  // func_27215_d

    bool        operator==(const J_JsonArray& other) const;
    std::string toString() const;

private:
    std::vector<J_JsonNode*> elements;  // field_27221_a
};
