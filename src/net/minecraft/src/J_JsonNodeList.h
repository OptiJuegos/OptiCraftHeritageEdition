#pragma once

#include <vector>

#include "J_JsonNode.h"

// net.minecraft.src.J_JsonNodeList
// Thin ArrayList subclass in Java; in C++ just a vector built from an iterable.
class J_JsonNodeList : public std::vector<J_JsonNode*>
{
public:
    explicit J_JsonNodeList(const std::vector<J_JsonNode*>& src)
        : std::vector<J_JsonNode*>(src)
    {
    }
};
