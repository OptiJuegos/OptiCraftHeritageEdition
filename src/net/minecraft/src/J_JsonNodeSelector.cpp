#include "J_JsonNodeSelector.h"
#include "J_ChainedFunctor.h"

J_JsonNodeSelector::J_JsonNodeSelector(J_Functor* functor)
    : functor(functor)
{
}

J_JsonNodeSelector::~J_JsonNodeSelector()
{
    delete functor;
}

bool J_JsonNodeSelector::matches(const SelectorValue& val)
{
    return functor->matches(val);
}

SelectorValue J_JsonNodeSelector::getValue(const SelectorValue& val)
{
    return functor->getValue(val);
}

J_JsonNodeSelector* J_JsonNodeSelector::chain(J_JsonNodeSelector* next)
{
    return new J_JsonNodeSelector(new J_ChainedFunctor(this, next));
}

std::string J_JsonNodeSelector::getDescription()
{
    return functor->getName();
}

std::string J_JsonNodeSelector::toString()
{
    return functor->toString();
}
