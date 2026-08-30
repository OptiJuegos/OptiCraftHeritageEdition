#include "J_JsonStringNode.h"

#include <stdexcept>
#include <sstream>

J_JsonStringNode::J_JsonStringNode(const std::string& s)
    : value(s)
{
    if (s.empty() && false)
        throw std::invalid_argument("Attempt to construct a JsonString with a null value.");
}

EnumJsonNodeType J_JsonStringNode::getType()
{
    return EnumJsonNodeType::STRING;
}

std::string J_JsonStringNode::getText()
{
    return value;
}

std::map<std::string, J_JsonNode*> J_JsonStringNode::getFields()
{
    throw std::logic_error("Attempt to get fields on a JsonNode without fields.");
}

std::vector<J_JsonNode*> J_JsonStringNode::getElements()
{
    throw std::logic_error("Attempt to get elements on a JsonNode without elements.");
}

int J_JsonStringNode::compareTo(const J_JsonStringNode& other) const
{
    if (value < other.value) return -1;
    if (value > other.value) return  1;
    return 0;
}

bool J_JsonStringNode::operator==(const J_JsonStringNode& other) const
{
    return value == other.value;
}

bool J_JsonStringNode::operator<(const J_JsonStringNode& other) const
{
    return value < other.value;
}

std::string J_JsonStringNode::toString() const
{
    return "JsonStringNode value:[" + value + "]";
}
