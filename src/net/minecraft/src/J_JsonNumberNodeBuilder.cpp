#include "J_JsonNumberNodeBuilder.h"
#include "J_JsonNodeFactories.h"

J_JsonNumberNodeBuilder::J_JsonNumberNodeBuilder(const std::string& s)
    : node(J_JsonNodeFactories::numberNode(s))
{
}

J_JsonNode* J_JsonNumberNodeBuilder::build()
{
    return node;
}
