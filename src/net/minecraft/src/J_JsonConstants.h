#pragma once

#include <string>
#include <map>
#include <vector>
#include <stdexcept>

#include "J_JsonNode.h"
#include "EnumJsonNodeType.h"

// net.minecraft.src.J_JsonConstants
// Singleton instances for JSON null, true, and false constants.
class J_JsonConstants : public J_JsonNode
{
public:
    EnumJsonNodeType                    getType()     override;  // func_27218_a
    std::string                         getText()     override;  // func_27216_b
    std::map<std::string, J_JsonNode*>  getFields()   override;  // func_27214_c
    std::vector<J_JsonNode*>            getElements() override;  // func_27215_d

    static J_JsonConstants JSON_NULL;   // field_27228_a
    static J_JsonConstants JSON_TRUE;   // field_27227_b
    static J_JsonConstants JSON_FALSE;  // field_27230_c

private:
    explicit J_JsonConstants(EnumJsonNodeType type);

    EnumJsonNodeType nodeType;  // field_27229_d
};
