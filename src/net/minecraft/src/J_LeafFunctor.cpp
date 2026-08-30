#include "J_LeafFunctor.h"
#include "J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException.h"

SelectorValue J_LeafFunctor::getValue(const SelectorValue& val)
{
    if (!matches(val))
        throw J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException::createForFunctor(this);
    return extractValue(val);
}
