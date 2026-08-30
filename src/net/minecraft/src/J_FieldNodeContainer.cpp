#include "J_FieldNodeContainer.h"
#include "J_JsonFieldBuilder.h"
#include <stdexcept>

J_FieldNodeContainer::J_FieldNodeContainer(J_JsonListenerToJdomAdapter* adapter, J_JsonFieldBuilder* fieldBuilder)
    : adapter(adapter)
    , fieldBuilder(fieldBuilder)
{
}

void J_FieldNodeContainer::addNodeBuilder(J_JsonNodeBuilder* builder)
{
    fieldBuilder->setValue(builder);
}

void J_FieldNodeContainer::addFieldBuilder(J_JsonFieldBuilder* field)
{
    throw std::runtime_error("Coding failure in Argo:  Attempt to add a field to a field.");
}
