#pragma once

#include "J_NodeContainer.h"

class J_JsonListenerToJdomAdapter;
class J_JsonFieldBuilder;

// net.minecraft.src.J_FieldNodeContainer
// Node container for when we are inside an object field value.
class J_FieldNodeContainer : public J_NodeContainer
{
public:
    J_FieldNodeContainer(J_JsonListenerToJdomAdapter* adapter, J_JsonFieldBuilder* fieldBuilder);

    void addNodeBuilder(J_JsonNodeBuilder* builder) override;   // func_27290_a
    void addFieldBuilder(J_JsonFieldBuilder* field) override;   // func_27289_a

private:
    J_JsonFieldBuilder*          fieldBuilder;  // field_27292_a
    J_JsonListenerToJdomAdapter* adapter;       // field_27291_b
};
