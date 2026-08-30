#include "J_JsonFieldNodeSelector.h"

J_JsonFieldNodeSelector::J_JsonFieldNodeSelector(const std::string& fieldName)
    : fieldName(fieldName)
{
}

bool J_JsonFieldNodeSelector::matches(const SelectorValue& val)
{
    return val.kind == SelectorValue::OBJ
        && val.obj.find(fieldName) != val.obj.end();
}

std::string J_JsonFieldNodeSelector::getName()
{
    return "\"" + fieldName + "\"";
}

SelectorValue J_JsonFieldNodeSelector::extractValue(const SelectorValue& val)
{
    return SelectorValue(val.obj.at(fieldName));
}

std::string J_JsonFieldNodeSelector::toString()
{
    return "a field called [\"" + fieldName + "\"]";
}
