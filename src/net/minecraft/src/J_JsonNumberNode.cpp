#include "J_JsonNumberNode.h"

#include <stdexcept>
#include <cctype>

// Validates JSON number format: (-?)(0|[1-9][0-9]*)(\.[0-9]+)?((e|E)(\+|-)?[0-9]+)?
bool J_JsonNumberNode::isValidNumber(const std::string& s)
{
    if (s.empty())
        return false;

    size_t i = 0;

    if (s[i] == '-')
        i++;

    if (i >= s.size())
        return false;

    if (s[i] == '0')
    {
        i++;
    }
    else if (s[i] >= '1' && s[i] <= '9')
    {
        i++;
        while (i < s.size() && std::isdigit((unsigned char)s[i]))
            i++;
    }
    else
    {
        return false;
    }

    if (i < s.size() && s[i] == '.')
    {
        i++;
        if (i >= s.size() || !std::isdigit((unsigned char)s[i]))
            return false;
        while (i < s.size() && std::isdigit((unsigned char)s[i]))
            i++;
    }

    if (i < s.size() && (s[i] == 'e' || s[i] == 'E'))
    {
        i++;
        if (i < s.size() && (s[i] == '+' || s[i] == '-'))
            i++;
        if (i >= s.size() || !std::isdigit((unsigned char)s[i]))
            return false;
        while (i < s.size() && std::isdigit((unsigned char)s[i]))
            i++;
    }

    return i == s.size();
}

J_JsonNumberNode::J_JsonNumberNode(const std::string& s)
    : value(s)
{
    if (s.empty())
        throw std::invalid_argument("Attempt to construct a JsonNumber with a null value.");
    if (!isValidNumber(s))
        throw std::invalid_argument(
            "Attempt to construct a JsonNumber with a String [" + s +
            "] that does not match the JSON number specification.");
}

EnumJsonNodeType J_JsonNumberNode::getType()
{
    return EnumJsonNodeType::NUMBER;
}

std::string J_JsonNumberNode::getText()
{
    return value;
}

std::map<std::string, J_JsonNode*> J_JsonNumberNode::getFields()
{
    throw std::logic_error("Attempt to get fields on a JsonNode without fields.");
}

std::vector<J_JsonNode*> J_JsonNumberNode::getElements()
{
    throw std::logic_error("Attempt to get elements on a JsonNode without elements.");
}

bool J_JsonNumberNode::operator==(const J_JsonNumberNode& other) const
{
    return value == other.value;
}

std::string J_JsonNumberNode::toString() const
{
    return "JsonNumberNode value:[" + value + "]";
}
