#include "J_JsonStringNodeSelector.h"
#include "J_JsonNode.h"
#include "EnumJsonNodeType.h"

bool J_JsonStringNodeSelector::matches(const SelectorValue& val)
{
    return val.kind == SelectorValue::NODE
        && val.node != nullptr
        && EnumJsonNodeType::STRING == val.node->getType();
}

std::string J_JsonStringNodeSelector::getName()
{
    return "A short form string";
}

SelectorValue J_JsonStringNodeSelector::extractValue(const SelectorValue& val)
{
    return SelectorValue(val.node->getText());
}

std::string J_JsonStringNodeSelector::toString()
{
    return "a value that is a string";
}
