#include "J_JsonArrayNodeSelector.h"
#include "J_JsonNode.h"
#include "EnumJsonNodeType.h"

bool J_JsonArrayNodeSelector::matches(const SelectorValue& val)
{
    return val.kind == SelectorValue::NODE
        && val.node != nullptr
        && EnumJsonNodeType::ARRAY == val.node->getType();
}

std::string J_JsonArrayNodeSelector::getName()
{
    return "A short form array";
}

SelectorValue J_JsonArrayNodeSelector::extractValue(const SelectorValue& val)
{
    return SelectorValue(val.node->getElements());
}

std::string J_JsonArrayNodeSelector::toString()
{
    return "an array";
}
