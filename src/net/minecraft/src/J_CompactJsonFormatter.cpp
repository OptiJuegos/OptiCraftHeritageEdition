#include "J_CompactJsonFormatter.h"
#include "J_JsonNode.h"
#include "J_JsonRootNode.h"
#include "J_JsonEscapedString.h"
#include "EnumJsonNodeType.h"
#include <stdexcept>

std::string J_CompactJsonFormatter::format(J_JsonRootNode* root)
{
    std::ostringstream oss;
    format(root, oss);
    return oss.str();
}

void J_CompactJsonFormatter::format(J_JsonRootNode* root, std::ostream& out)
{
    formatNode(root, out);
}

void J_CompactJsonFormatter::formatNode(J_JsonNode* node, std::ostream& out)
{
    switch (node->getType())
    {
    case EnumJsonNodeType::ARRAY:
    {
        out << '[';
        bool first = true;
        for (J_JsonNode* elem : node->getElements())
        {
            if (!first)
                out << ',';
            first = false;
            formatNode(elem, out);
        }
        out << ']';
        break;
    }

    case EnumJsonNodeType::OBJECT:
    {
        out << '{';
        bool first = true;
        for (auto& kv : node->getFields())
        {
            if (!first)
                out << ',';
            first = false;
            // write key as escaped JSON string
            out << '"' << J_JsonEscapedString(kv.first).toString() << '"';
            out << ':';
            formatNode(kv.second, out);
        }
        out << '}';
        break;
    }

    case EnumJsonNodeType::STRING:
        out << '"' << J_JsonEscapedString(node->getText()).toString() << '"';
        break;

    case EnumJsonNodeType::NUMBER:
        out << node->getText();
        break;

    case EnumJsonNodeType::FALSE:
        out << "false";
        break;

    case EnumJsonNodeType::TRUE:
        out << "true";
        break;

    case EnumJsonNodeType::NULL_:
        out << "null";
        break;

    default:
        throw std::runtime_error(
            std::string("Coding failure in Argo:  Attempt to format a JsonNode of unknown type."));
    }
}
