#pragma once

#include <istream>
#include <sstream>
#include <string>

#include "J_InvalidSyntaxException.h"

class J_JsonRootNode;

// net.minecraft.src.J_JdomParser
// High-level parser that produces a J_JsonRootNode DOM from a stream or string.
class J_JdomParser
{
public:
    J_JdomParser() {}

    // func_27366_a - parse from an input stream
    J_JsonRootNode* parse(std::istream& in);

    // func_27367_a - parse from a string
    J_JsonRootNode* parse(const std::string& s);
};
