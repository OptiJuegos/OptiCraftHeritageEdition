#pragma once

#include <list>

#include "J_JsonNodeBuilder.h"

class J_JsonRootNode;

// net.minecraft.src.J_JsonArrayNodeBuilder
class J_JsonArrayNodeBuilder : public J_JsonNodeBuilder
{
public:
    J_JsonArrayNodeBuilder() {}

    // func_27240_a - add an element builder
    J_JsonArrayNodeBuilder* addElement(J_JsonNodeBuilder* element);

    // func_27241_a - build the J_JsonRootNode (a J_JsonArray)
    J_JsonRootNode* buildArray();

    J_JsonNode* build() override;  // func_27234_b

private:
    std::list<J_JsonNodeBuilder*> elements;  // field_27242_a
};
