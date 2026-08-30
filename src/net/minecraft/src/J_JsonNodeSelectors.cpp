#include "J_JsonNodeSelectors.h"
#include "J_JsonArrayNodeSelector.h"
#include "J_JsonStringNodeSelector.h"
#include "J_JsonObjectNodeSelector.h"
#include "J_JsonFieldNodeSelector.h"
#include "J_JsonElementNodeSelector.h"
#include "J_ChainedFunctor.h"
#include <stdexcept>

J_JsonNodeSelector* J_JsonNodeSelectors::stringAt(const std::vector<JsonPathElement>& path)
{
    return buildChain(path, new J_JsonNodeSelector(new J_JsonStringNodeSelector()));
}

J_JsonNodeSelector* J_JsonNodeSelectors::arrayAt(const std::vector<JsonPathElement>& path)
{
    return buildChain(path, new J_JsonNodeSelector(new J_JsonArrayNodeSelector()));
}

J_JsonNodeSelector* J_JsonNodeSelectors::objectAt(const std::vector<JsonPathElement>& path)
{
    return buildChain(path, new J_JsonNodeSelector(new J_JsonObjectNodeSelector()));
}

J_JsonNodeSelector* J_JsonNodeSelectors::fieldCalled(const std::string& name)
{
    return new J_JsonNodeSelector(new J_JsonFieldNodeSelector(name));
}

J_JsonNodeSelector* J_JsonNodeSelectors::objectFieldCalled(const std::string& name)
{
    return chainSelectors(
        objectAt({}),
        fieldCalled(name));
}

J_JsonNodeSelector* J_JsonNodeSelectors::elementAt(int index)
{
    return new J_JsonNodeSelector(new J_JsonElementNodeSelector(index));
}

J_JsonNodeSelector* J_JsonNodeSelectors::arrayElementAt(int index)
{
    return chainSelectors(
        arrayAt({}),
        elementAt(index));
}

J_JsonNodeSelector* J_JsonNodeSelectors::buildChain(
    const std::vector<JsonPathElement>& path,
    J_JsonNodeSelector* tail)
{
    J_JsonNodeSelector* result = tail;
    for (int i = (int)path.size() - 1; i >= 0; i--)
    {
        const JsonPathElement& elem = path[i];
        if (elem.type == JsonPathElement::INT)
        {
            result = chainSelectors(arrayElementAt(elem.intVal), result);
        }
        else if (elem.type == JsonPathElement::STR)
        {
            result = chainSelectors(objectFieldCalled(elem.str), result);
        }
        else
        {
            throw std::invalid_argument("Unknown JsonPathElement type in J_JsonNodeSelectors::buildChain");
        }
    }
    return result;
}

J_JsonNodeSelector* J_JsonNodeSelectors::chainSelectors(
    J_JsonNodeSelector* head,
    J_JsonNodeSelector* tail)
{
    return new J_JsonNodeSelector(new J_ChainedFunctor(head, tail));
}
