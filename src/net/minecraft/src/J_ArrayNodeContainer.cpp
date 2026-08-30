#include "J_ArrayNodeContainer.h"
#include "J_JsonArrayNodeBuilder.h"
#include <stdexcept>

J_ArrayNodeContainer::J_ArrayNodeContainer(J_JsonListenerToJdomAdapter* adapter, J_JsonArrayNodeBuilder* arrayBuilder)
    : adapter(adapter)
    , arrayBuilder(arrayBuilder)
{
}

void J_ArrayNodeContainer::addNodeBuilder(J_JsonNodeBuilder* builder)
{
    arrayBuilder->addElement(builder);
}

void J_ArrayNodeContainer::addFieldBuilder(J_JsonFieldBuilder* field)
{
    throw std::runtime_error("Coding failure in Argo:  Attempt to add a field to an array.");
}
