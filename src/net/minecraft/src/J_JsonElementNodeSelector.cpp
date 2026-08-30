#include "J_JsonElementNodeSelector.h"

J_JsonElementNodeSelector::J_JsonElementNodeSelector(int index)
    : index(index)
{
}

bool J_JsonElementNodeSelector::matches(const SelectorValue& val)
{
    return val.kind == SelectorValue::ARR
        && (int)val.arr.size() > index;
}

std::string J_JsonElementNodeSelector::getName()
{
    return std::to_string(index);
}

SelectorValue J_JsonElementNodeSelector::extractValue(const SelectorValue& val)
{
    return SelectorValue(val.arr[index]);
}

std::string J_JsonElementNodeSelector::toString()
{
    return "an element at index [" + std::to_string(index) + "]";
}
