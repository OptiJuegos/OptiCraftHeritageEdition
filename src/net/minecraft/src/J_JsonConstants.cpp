#include "J_JsonConstants.h"

J_JsonConstants J_JsonConstants::JSON_NULL (EnumJsonNodeType::NULL_);
J_JsonConstants J_JsonConstants::JSON_TRUE (EnumJsonNodeType::TRUE);
J_JsonConstants J_JsonConstants::JSON_FALSE(EnumJsonNodeType::FALSE);

J_JsonConstants::J_JsonConstants(EnumJsonNodeType type)
    : nodeType(type)
{
}

EnumJsonNodeType J_JsonConstants::getType()
{
    return nodeType;
}

std::string J_JsonConstants::getText()
{
    throw std::logic_error("Attempt to get text on a JsonNode without text.");
}

std::map<std::string, J_JsonNode*> J_JsonConstants::getFields()
{
    throw std::logic_error("Attempt to get fields on a JsonNode without fields.");
}

std::vector<J_JsonNode*> J_JsonConstants::getElements()
{
    throw std::logic_error("Attempt to get elements on a JsonNode without elements.");
}
