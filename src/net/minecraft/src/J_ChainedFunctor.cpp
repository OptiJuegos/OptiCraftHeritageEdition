#include "J_ChainedFunctor.h"
#include "J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException.h"

J_ChainedFunctor::J_ChainedFunctor(J_JsonNodeSelector* head, J_JsonNodeSelector* tail)
    : head(head)
    , tail(tail)
{
}

bool J_ChainedFunctor::matches(const SelectorValue& val)
{
    return head->matches(val) && tail->matches(head->getValue(val));
}

SelectorValue J_ChainedFunctor::getValue(const SelectorValue& val)
{
    SelectorValue mid = [&]() -> SelectorValue {
        try
        {
            return head->getValue(val);
        }
        catch (J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& e)
        {
            throw J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException::replaceWithOuter(e, head);
        }
    }();

    try
    {
        return tail->getValue(mid);
    }
    catch (J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& e)
    {
        throw J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException::wrapWithOuter(e, head);
    }
}

std::string J_ChainedFunctor::getName()
{
    return tail->getDescription();
}

std::string J_ChainedFunctor::toString()
{
    return head->toString() + ", with " + tail->toString();
}
