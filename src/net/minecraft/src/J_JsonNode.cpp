#include "J_JsonNode.h"
#include "J_JsonNodeSelectors.h"
#include "J_JsonNodeFactories.h"
#include "J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException.h"
#include "J_JsonNodeDoesNotMatchPathElementsException.h"
#include "J_JsonNodeSelector.h"
#include "J_JsonRootNode.h"

#include <memory>

// func_27219_a - navigate a selector and throw a detailed path exception on failure
static SelectorValue navigateNode(J_JsonNodeSelector* selector, J_JsonNode* node,
    const std::vector<JsonPathElement>& path)
{
    try
    {
        return selector->getValue(SelectorValue(node));
    }
    catch (J_JsonNodeDoesNotMatchChainedJsonNodeSelectorException& e)
    {
        J_JsonNode* arr[1] = { node };
        J_JsonRootNode* root = J_JsonNodeFactories::arrayNode(arr, 1);
        throw J_JsonNodeDoesNotMatchPathElementsException::create(e, path, root);
    }
}

// func_27213_a
std::string J_JsonNode::getStringAt(const std::vector<JsonPathElement>& path)
{
    std::unique_ptr<J_JsonNodeSelector> sel(J_JsonNodeSelectors::stringAt(path));
    SelectorValue result = navigateNode(sel.get(), this, path);
    return result.str;
}

// func_27217_b
std::vector<J_JsonNode*> J_JsonNode::getArrayAt(const std::vector<JsonPathElement>& path)
{
    std::unique_ptr<J_JsonNodeSelector> sel(J_JsonNodeSelectors::arrayAt(path));
    SelectorValue result = navigateNode(sel.get(), this, path);
    return result.arr;
}
