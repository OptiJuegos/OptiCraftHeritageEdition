#include "J_JsonFieldBuilder.h"
#include "J_JsonStringNode.h"

J_JsonFieldBuilder* J_JsonFieldBuilder::create()
{
    return new J_JsonFieldBuilder();
}

J_JsonFieldBuilder* J_JsonFieldBuilder::setKey(J_JsonNodeBuilder* builder)
{
    keyBuilder = builder;
    return this;
}

J_JsonFieldBuilder* J_JsonFieldBuilder::setValue(J_JsonNodeBuilder* builder)
{
    valueBuilder = builder;
    return this;
}

J_JsonStringNode* J_JsonFieldBuilder::getKeyNode()
{
    return static_cast<J_JsonStringNode*>(keyBuilder->build());
}

J_JsonNode* J_JsonFieldBuilder::getValueNode()
{
    return valueBuilder->build();
}
