#pragma once

#include <string>

class J_JsonRootNode;

// net.minecraft.src.J_JsonFormatter
class J_JsonFormatter
{
public:
    virtual ~J_JsonFormatter() {}
    virtual std::string format(J_JsonRootNode* root) = 0;  // func_27327_a
};
