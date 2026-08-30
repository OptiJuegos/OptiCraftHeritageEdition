#include "J_JsonArray.h"

J_JsonArray::J_JsonArray(const std::vector<J_JsonNode*>& elems)
    : elements(elems)
{
}

EnumJsonNodeType J_JsonArray::getType()
{
    return EnumJsonNodeType::ARRAY;
}

std::string J_JsonArray::getText()
{
    throw std::logic_error("Attempt to get text on a JsonNode without text.");
}

std::map<std::string, J_JsonNode*> J_JsonArray::getFields()
{
    throw std::logic_error("Attempt to get fields on a JsonNode without fields.");
}

std::vector<J_JsonNode*> J_JsonArray::getElements()
{
    return elements;
}

bool J_JsonArray::operator==(const J_JsonArray& other) const
{
    return elements == other.elements;
}

std::string J_JsonArray::toString() const
{
    return "JsonArray elements:[...]";
}
