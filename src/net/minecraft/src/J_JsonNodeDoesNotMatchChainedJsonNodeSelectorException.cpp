#include "J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException.h"
#include "J_JsonNodeSelector.h"

J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException::J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException(
    J_Functor* functor,
    const std::vector<J_JsonNodeSelector*>& path)
    : J_JsonNodeDoesNotMatchJsonNodeSelectorException(
        "Failed to match any JSON node at [" + formatPath(path) + "]")
    , failedNode(functor)
    , failPath(path)
{
}

J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException
J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException::createForFunctor(J_Functor* functor)
{
    return J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException(functor, {});
}

J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException
J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException::wrapWithOuter(
    const J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& inner,
    J_JsonNodeSelector* outerSelector)
{
    std::vector<J_JsonNodeSelector*> path = inner.failPath;
    path.push_back(outerSelector);
    return J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException(inner.failedNode, path);
}

J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException
J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException::replaceWithOuter(
    const J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& inner,
    J_JsonNodeSelector* outerSelector)
{
    std::vector<J_JsonNodeSelector*> path;
    path.push_back(outerSelector);
    return J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException(inner.failedNode, path);
}

std::string J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException::formatPath(
    const std::vector<J_JsonNodeSelector*>& path)
{
    std::string result;
    for (int i = (int)path.size() - 1; i >= 0; i--)
    {
        result += path[i]->getDescription();
        if (i != 0)
            result += ".";
    }
    return result;
}

std::string J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException::toString() const
{
    std::string result = "JsonNodeDoesNotMatchJsonNodeSelectorException{failedNode=";
    result += failedNode ? failedNode->getName() : "null";
    result += ", failPath=[";
    for (size_t i = 0; i < failPath.size(); i++)
    {
        if (i > 0) result += ", ";
        result += failPath[i]->getDescription();
    }
    result += "]}";
    return result;
}
