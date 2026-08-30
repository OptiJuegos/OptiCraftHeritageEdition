#pragma once

#include <stdexcept>
#include <string>

// net.minecraft.src.J_JsonNodeDoesNotMatchJsonNodeSelectorException
// Base exception for when a JSON node does not match a selector.
class J_JsonNodeDoesNotMatchJsonNodeSelectorException : public std::invalid_argument
{
public:
    explicit J_JsonNodeDoesNotMatchJsonNodeSelectorException(const std::string& msg)
        : std::invalid_argument(msg)
    {
    }
};
