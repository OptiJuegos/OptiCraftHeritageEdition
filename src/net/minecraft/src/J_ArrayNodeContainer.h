#pragma once

#include "J_NodeContainer.h"

class J_JsonListenerToJdomAdapter;
class J_JsonArrayNodeBuilder;

// net.minecraft.src.J_ArrayNodeContainer
// Node container for when we are currently building a JSON array.
class J_ArrayNodeContainer : public J_NodeContainer
{
public:
    J_ArrayNodeContainer(J_JsonListenerToJdomAdapter* adapter, J_JsonArrayNodeBuilder* arrayBuilder);

    void addNodeBuilder(J_JsonNodeBuilder* builder) override;   // func_27290_a
    void addFieldBuilder(J_JsonFieldBuilder* field) override;   // func_27289_a

private:
    J_JsonArrayNodeBuilder*      arrayBuilder;  // field_27294_a
    J_JsonListenerToJdomAdapter* adapter;       // field_27293_b
};
