#include "J_JsonNodeDoesNotMatchPathElementsException.h"
#include "J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException.h"
#include "J_CompactJsonFormatter.h"
#include "J_JsonRootNode.h"

J_JsonNodeDoesNotMatchPathElementsException::J_JsonNodeDoesNotMatchPathElementsException(
    const std::string& msg)
    : J_JsonNodeDoesNotMatchJsonNodeSelectorException(msg)
{
}

J_JsonNodeDoesNotMatchPathElementsException
J_JsonNodeDoesNotMatchPathElementsException::create(
    const J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& inner,
    const std::vector<JsonPathElement>& path,
    J_JsonRootNode* root)
{
    return J_JsonNodeDoesNotMatchPathElementsException(buildMessage(inner, path, root));
}

std::string J_JsonNodeDoesNotMatchPathElementsException::buildMessage(
    const J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& inner,
    const std::vector<JsonPathElement>& path,
    J_JsonRootNode* root)
{
    J_CompactJsonFormatter fmt;
    std::string result = "Failed to find ";
    result += inner.failedNode ? inner.failedNode->toString() : "null";
    result += " at [";
    result += J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException::formatPath(inner.failPath);
    result += "] while resolving [";
    result += formatPath(path);
    result += "] in ";
    result += fmt.format(root);
    result += ".";
    return result;
}

std::string J_JsonNodeDoesNotMatchPathElementsException::formatPath(
    const std::vector<JsonPathElement>& path)
{
    std::string result;
    bool first = true;
    for (const auto& elem : path)
    {
        if (!first)
            result += ".";
        first = false;
        if (elem.type == JsonPathElement::STR)
            result += "\"" + elem.str + "\"";
        else
            result += std::to_string(elem.intVal);
    }
    return result;
}
