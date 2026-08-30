#pragma once

#include <map>
#include <string>

#include "J_JsonNode.h"
#include "J_JsonObjectNodeBuilder.h"
#include "J_JsonFieldBuilder.h"

// net.minecraft.src.J_JsonObjectNodeList
// HashMap subclass in Java; in C++ a map built from J_JsonObjectNodeBuilder's field list.
class J_JsonObjectNodeList : public std::map<std::string, J_JsonNode*>
{
public:
    explicit J_JsonObjectNodeList(J_JsonObjectNodeBuilder* builder);
};
