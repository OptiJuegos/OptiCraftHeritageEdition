#pragma once

#include "J_NodeContainer.h"

class J_JsonListenerToJdomAdapter;
class J_JsonObjectNodeBuilder;

// net.minecraft.src.J_ObjectNodeContainer
// Node container for when we are currently building a JSON object.
class J_ObjectNodeContainer : public J_NodeContainer
{
public:
    J_ObjectNodeContainer(J_JsonListenerToJdomAdapter* adapter, J_JsonObjectNodeBuilder* objectBuilder);

    void addNodeBuilder(J_JsonNodeBuilder* builder) override;   // func_27290_a
    void addFieldBuilder(J_JsonFieldBuilder* field) override;   // func_27289_a

private:
    J_JsonObjectNodeBuilder*     objectBuilder;  // field_27296_a
    J_JsonListenerToJdomAdapter* adapter;        // field_27295_b
};
