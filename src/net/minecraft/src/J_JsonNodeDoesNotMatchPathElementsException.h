#pragma once

#include <string>
#include <vector>

#include "J_JsonNodeDoesNotMatchJsonNodeSelectorException.h"
#include "J_JsonNode.h"

class J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException;
class J_JsonRootNode;

// net.minecraft.src.J_JsonNodeDoesNotMatchPathElementsException
class J_JsonNodeDoesNotMatchPathElementsException
    : public J_JsonNodeDoesNotMatchJsonNodeSelectorException
{
public:
    // func_27319_a - factory
    static J_JsonNodeDoesNotMatchPathElementsException create(
        const J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& inner,
        const std::vector<JsonPathElement>& path,
        J_JsonRootNode* root);

private:
    explicit J_JsonNodeDoesNotMatchPathElementsException(const std::string& msg);

    static std::string buildMessage(
        const J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& inner,
        const std::vector<JsonPathElement>& path,
        J_JsonRootNode* root);

    static std::string formatPath(const std::vector<JsonPathElement>& path);  // func_27317_a
};
