#pragma once

#include <string>
#include <vector>
#include <map>

class J_JsonNode;
class J_JsonRootNode;
class J_JsonStringNode;

// net.minecraft.src.J_JsonNodeFactories
// Static factory methods for creating JSON node instances.
class J_JsonNodeFactories
{
public:
    // func_27310_a - null constant
    static J_JsonNode* nullNode();

    // func_27313_b - true constant
    static J_JsonNode* trueNode();

    // func_27314_c - false constant
    static J_JsonNode* falseNode();

    // func_27316_a - new string node
    static J_JsonStringNode* stringNode(const std::string& s);

    // func_27311_b - new number node
    static J_JsonNode* numberNode(const std::string& s);

    // func_27309_a - new array from element list
    static J_JsonRootNode* arrayNode(const std::vector<J_JsonNode*>& elements);

    // func_27315_a - new array from C array
    static J_JsonRootNode* arrayNode(J_JsonNode** nodes, int count);

    // func_27312_a - new object from field map (string key -> node value)
    static J_JsonRootNode* objectNode(const std::map<std::string, J_JsonNode*>& fields);

private:
    J_JsonNodeFactories() {}
};
