#pragma once

#include "J_JsonNodeBuilder.h"

class J_JsonStringNode;
class J_JsonNode;

// net.minecraft.src.J_JsonFieldBuilder
// Holds a key builder and a value builder for one object field.
class J_JsonFieldBuilder
{
public:
    // func_27301_a
    static J_JsonFieldBuilder* create();

    // func_27304_a - set the key builder (must produce a J_JsonStringNode)
    J_JsonFieldBuilder* setKey(J_JsonNodeBuilder* keyBuilder);

    // func_27300_b - set the value builder
    J_JsonFieldBuilder* setValue(J_JsonNodeBuilder* valueBuilder);

    // func_27303_b - build the key node (cast to J_JsonStringNode)
    J_JsonStringNode* getKeyNode();

    // func_27302_c - build the value node
    J_JsonNode* getValueNode();

private:
    J_JsonFieldBuilder() : keyBuilder(nullptr), valueBuilder(nullptr) {}

    J_JsonNodeBuilder* keyBuilder;    // field_27306_a
    J_JsonNodeBuilder* valueBuilder;  // field_27305_b
};
