#include "J_JsonArrayNodeBuilder.h"
#include "J_JsonNodeFactories.h"
#include "J_JsonRootNode.h"

J_JsonArrayNodeBuilder* J_JsonArrayNodeBuilder::addElement(J_JsonNodeBuilder* element)
{
    elements.push_back(element);
    return this;
}

J_JsonRootNode* J_JsonArrayNodeBuilder::buildArray()
{
    std::vector<J_JsonNode*> elems;
    for (J_JsonNodeBuilder* b : elements)
        elems.push_back(b->build());
    return J_JsonNodeFactories::arrayNode(elems);
}

J_JsonNode* J_JsonArrayNodeBuilder::build()
{
    return buildArray();
}
