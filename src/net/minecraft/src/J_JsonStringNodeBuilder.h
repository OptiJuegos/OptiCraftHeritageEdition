#pragma once

#include <string>

#include "J_JsonNodeBuilder.h"

class J_JsonStringNode;

// net.minecraft.src.J_JsonStringNodeBuilder
class J_JsonStringNodeBuilder : public J_JsonNodeBuilder
{
public:
    explicit J_JsonStringNodeBuilder(const std::string& s);

    // func_27243_a - build a J_JsonStringNode specifically
    J_JsonStringNode* buildString();

    J_JsonNode* build() override;  // func_27234_b

private:
    std::string value;  // field_27244_a
};
