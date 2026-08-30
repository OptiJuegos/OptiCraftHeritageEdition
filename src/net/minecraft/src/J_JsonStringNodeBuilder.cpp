#include "J_JsonStringNodeBuilder.h"
#include "J_JsonNodeFactories.h"
#include "J_JsonStringNode.h"

J_JsonStringNodeBuilder::J_JsonStringNodeBuilder(const std::string& s)
    : value(s)
{
}

J_JsonStringNode* J_JsonStringNodeBuilder::buildString()
{
    return J_JsonNodeFactories::stringNode(value);
}

J_JsonNode* J_JsonStringNodeBuilder::build()
{
    return buildString();
}
