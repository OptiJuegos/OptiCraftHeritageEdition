#pragma once

class J_JsonNodeBuilder;
class J_JsonFieldBuilder;

// net.minecraft.src.J_NodeContainer
// Interface for the current node context during SAX-style parsing.
class J_NodeContainer
{
public:
    virtual ~J_NodeContainer() {}

    // func_27290_a - add a node builder to this container
    virtual void addNodeBuilder(J_JsonNodeBuilder* builder) = 0;

    // func_27289_a - add a field builder to this container
    virtual void addFieldBuilder(J_JsonFieldBuilder* field) = 0;
};
