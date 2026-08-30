#include "J_JsonNodeFactories.h"
#include "J_JsonConstants.h"
#include "J_JsonStringNode.h"
#include "J_JsonNumberNode.h"
#include "J_JsonArray.h"
#include "J_JsonObject.h"

J_JsonNode* J_JsonNodeFactories::nullNode()
{
    return &J_JsonConstants::JSON_NULL;
}

J_JsonNode* J_JsonNodeFactories::trueNode()
{
    return &J_JsonConstants::JSON_TRUE;
}

J_JsonNode* J_JsonNodeFactories::falseNode()
{
    return &J_JsonConstants::JSON_FALSE;
}

J_JsonStringNode* J_JsonNodeFactories::stringNode(const std::string& s)
{
    return new J_JsonStringNode(s);
}

J_JsonNode* J_JsonNodeFactories::numberNode(const std::string& s)
{
    return new J_JsonNumberNode(s);
}

J_JsonRootNode* J_JsonNodeFactories::arrayNode(const std::vector<J_JsonNode*>& elements)
{
    return new J_JsonArray(elements);
}

J_JsonRootNode* J_JsonNodeFactories::arrayNode(J_JsonNode** nodes, int count)
{
    std::vector<J_JsonNode*> elems(nodes, nodes + count);
    return new J_JsonArray(elems);
}

J_JsonRootNode* J_JsonNodeFactories::objectNode(const std::map<std::string, J_JsonNode*>& fields)
{
    return new J_JsonObject(fields);
}
