#include "J_ObjectNodeContainer.h"
#include "J_JsonObjectNodeBuilder.h"
#include "J_JsonFieldBuilder.h"
#include <stdexcept>

J_ObjectNodeContainer::J_ObjectNodeContainer(J_JsonListenerToJdomAdapter* adapter, J_JsonObjectNodeBuilder* objectBuilder)
    : adapter(adapter)
    , objectBuilder(objectBuilder)
{
}

void J_ObjectNodeContainer::addNodeBuilder(J_JsonNodeBuilder* builder)
{
    throw std::runtime_error("Coding failure in Argo:  Attempt to add a node to an object.");
}

void J_ObjectNodeContainer::addFieldBuilder(J_JsonFieldBuilder* field)
{
    objectBuilder->addField(field);
}
