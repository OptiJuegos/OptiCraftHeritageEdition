#include "J_JsonObjectNodeBuilder.h"
#include "J_JsonObjectNodeList.h"
#include "J_JsonNodeFactories.h"
#include "J_JsonRootNode.h"

J_JsonObjectNodeBuilder* J_JsonObjectNodeBuilder::addField(J_JsonFieldBuilder* field)
{
    fields.push_back(field);
    return this;
}

J_JsonRootNode* J_JsonObjectNodeBuilder::buildObject()
{
    J_JsonObjectNodeList nodeList(this);
    return J_JsonNodeFactories::objectNode(nodeList);
}

J_JsonNode* J_JsonObjectNodeBuilder::build()
{
    return buildObject();
}

const std::list<J_JsonFieldBuilder*>& J_JsonObjectNodeBuilder::getFields(J_JsonObjectNodeBuilder* builder)
{
    return builder->fields;
}
