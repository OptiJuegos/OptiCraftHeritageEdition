#pragma once

#include <list>

#include "J_JsonNodeBuilder.h"

class J_JsonFieldBuilder;
class J_JsonRootNode;

// net.minecraft.src.J_JsonObjectNodeBuilder
class J_JsonObjectNodeBuilder : public J_JsonNodeBuilder
{
public:
    J_JsonObjectNodeBuilder() {}

    // func_27237_a - add a field to this object builder
    J_JsonObjectNodeBuilder* addField(J_JsonFieldBuilder* field);

    // func_27235_a - build the J_JsonRootNode (a J_JsonObject)
    J_JsonRootNode* buildObject();

    J_JsonNode* build() override;  // func_27234_b

    // func_27236_a - access the internal field list (used by J_JsonObjectNodeList)
    static const std::list<J_JsonFieldBuilder*>& getFields(J_JsonObjectNodeBuilder* builder);

private:
    std::list<J_JsonFieldBuilder*> fields;  // field_27238_a
};
