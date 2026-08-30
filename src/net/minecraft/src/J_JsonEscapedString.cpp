#include "J_JsonEscapedString.h"

J_JsonEscapedString::J_JsonEscapedString(const std::string& s)
{
    value = replaceAll(s,      "\\", "\\\\");
    value = replaceAll(value,  "\"", "\\\"");
    value = replaceAll(value,  "\b", "\\b");
    value = replaceAll(value,  "\f", "\\f");
    value = replaceAll(value,  "\n", "\\n");
    value = replaceAll(value,  "\r", "\\r");
    value = replaceAll(value,  "\t", "\\t");
}

std::string J_JsonEscapedString::toString() const
{
    return value;
}

std::string J_JsonEscapedString::replaceAll(std::string str, const std::string& from, const std::string& to)
{
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos)
    {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}
