#pragma once

#include "J_JsonNodeBuilder.h"
#include "J_JsonNodeFactories.h"

// net.minecraft.src.J_JsonNullNodeBuilder
class J_JsonNullNodeBuilder : public J_JsonNodeBuilder
{
public:
    J_JsonNullNodeBuilder() {}

    J_JsonNode* build() override  // func_27234_b
    {
        return J_JsonNodeFactories::nullNode();
    }
};
