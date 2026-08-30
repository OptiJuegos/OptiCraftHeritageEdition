#pragma once

class J_JsonNode;

// net.minecraft.src.J_JsonNodeBuilder
class J_JsonNodeBuilder
{
public:
    virtual ~J_JsonNodeBuilder() {}
    virtual J_JsonNode* build() = 0;  // func_27234_b
};
