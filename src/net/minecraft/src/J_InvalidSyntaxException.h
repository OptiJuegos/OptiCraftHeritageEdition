#pragma once

#include <stdexcept>
#include <string>

#include "J_ThingWithPosition.h"

// net.minecraft.src.J_InvalidSyntaxException
class J_InvalidSyntaxException : public std::exception
{
public:
    J_InvalidSyntaxException(const std::string& msg, J_ThingWithPosition* pos)
        : column(pos->getColumn())
        , line(pos->getLine())
    {
        message = "At line " + std::to_string(line) + ", column " + std::to_string(column) + ":  " + msg;
    }

    J_InvalidSyntaxException(const std::string& msg, const std::exception& cause, J_ThingWithPosition* pos)
        : column(pos->getColumn())
        , line(pos->getLine())
    {
        message = "At line " + std::to_string(line) + ", column " + std::to_string(column) + ":  " + msg;
    }

    const char* what() const noexcept override { return message.c_str(); }

    int getColumn() const { return column; }
    int getLine()   const { return line; }

private:
    std::string message;
    int column;
    int line;
};
