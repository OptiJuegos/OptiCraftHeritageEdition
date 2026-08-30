#pragma once

#include <string>
#include <map>
#include <vector>

#include "J_JsonNode.h"
#include "EnumJsonNodeType.h"

// net.minecraft.src.J_JsonNumberNode
class J_JsonNumberNode : public J_JsonNode
{
public:
    explicit J_JsonNumberNode(const std::string& s);

    EnumJsonNodeType                    getType()     override;  // func_27218_a
    std::string                         getText()     override;  // func_27216_b
    std::map<std::string, J_JsonNode*>  getFields()   override;  // func_27214_c
    std::vector<J_JsonNode*>            getElements() override;  // func_27215_d

    bool        operator==(const J_JsonNumberNode& other) const;
    std::string toString() const;

private:
    static bool isValidNumber(const std::string& s);

    std::string value;  // field_27225_b
};
