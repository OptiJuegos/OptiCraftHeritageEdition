#pragma once

#include "J_JsonNodeBuilder.h"
#include "J_JsonNodeFactories.h"

// net.minecraft.src.J_JsonFalseNodeBuilder
class J_JsonFalseNodeBuilder : public J_JsonNodeBuilder
{
public:
    J_JsonFalseNodeBuilder() {}

    J_JsonNode* build() override  // func_27234_b
    {
        return J_JsonNodeFactories::falseNode();
    }
};
