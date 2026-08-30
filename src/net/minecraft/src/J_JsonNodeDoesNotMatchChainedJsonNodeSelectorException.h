#pragma once

#include <string>
#include <vector>

#include "J_JsonNodeDoesNotMatchJsonNodeSelectorException.h"
#include "J_Functor.h"

class J_JsonNodeSelector;

// net.minecraft.src.J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException
class J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException
    : public J_JsonNodeDoesNotMatchJsonNodeSelectorException
{
public:
    // func_27322_a - create for a functor that failed with no path yet
    static J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException createForFunctor(J_Functor* functor);

    // func_27323_a - wrap exception, appending outer selector to the path
    static J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException wrapWithOuter(
        const J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& inner,
        J_JsonNodeSelector* outerSelector);

    // func_27321_b - wrap exception, replacing path with just the outer selector
    static J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException replaceWithOuter(
        const J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& inner,
        J_JsonNodeSelector* outerSelector);

    // func_27324_a - format path list into dotted string
    static std::string formatPath(const std::vector<J_JsonNodeSelector*>& path);

    std::string toString() const;

    J_Functor*                       failedNode;  // field_27326_a
    std::vector<J_JsonNodeSelector*> failPath;    // field_27325_b

private:
    J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException(
        J_Functor* functor,
        const std::vector<J_JsonNodeSelector*>& path);
};
