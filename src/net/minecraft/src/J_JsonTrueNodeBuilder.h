#pragma once

#include "J_JsonNodeBuilder.h"
#include "J_JsonNodeFactories.h"

// net.minecraft.src.J_JsonTrueNodeBuilder
class J_JsonTrueNodeBuilder : public J_JsonNodeBuilder
{
public:
    J_JsonTrueNodeBuilder() {}

    J_JsonNode* build() override  // func_27234_b
    {
        return J_JsonNodeFactories::trueNode();
    }
};
