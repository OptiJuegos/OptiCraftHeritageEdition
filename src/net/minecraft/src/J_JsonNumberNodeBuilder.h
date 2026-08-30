#pragma once

#include <string>

#include "J_JsonNodeBuilder.h"

// net.minecraft.src.J_JsonNumberNodeBuilder
class J_JsonNumberNodeBuilder : public J_JsonNodeBuilder
{
public:
    explicit J_JsonNumberNodeBuilder(const std::string& s);

    J_JsonNode* build() override;  // func_27234_b

private:
    J_JsonNode* node;  // field_27239_a (pre-built at construction, matches Java)
};
