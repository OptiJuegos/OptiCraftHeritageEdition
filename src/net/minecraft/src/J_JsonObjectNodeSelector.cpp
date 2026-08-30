#include "J_JsonObjectNodeSelector.h"
#include "J_JsonNode.h"
#include "EnumJsonNodeType.h"

bool J_JsonObjectNodeSelector::matches(const SelectorValue& val)
{
    return val.kind == SelectorValue::NODE
        && val.node != nullptr
        && EnumJsonNodeType::OBJECT == val.node->getType();
}

std::string J_JsonObjectNodeSelector::getName()
{
    return "A short form object";
}

SelectorValue J_JsonObjectNodeSelector::extractValue(const SelectorValue& val)
{
    return SelectorValue(val.node->getFields());
}

std::string J_JsonObjectNodeSelector::toString()
{
    return "an object";
}
