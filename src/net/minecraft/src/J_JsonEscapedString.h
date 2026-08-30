#pragma once

#include <string>

// net.minecraft.src.J_JsonEscapedString
class J_JsonEscapedString
{
public:
    explicit J_JsonEscapedString(const std::string& s);

    std::string toString() const;

private:
    static std::string replaceAll(std::string str, const std::string& from, const std::string& to);

    std::string value;  // field_27031_a
};
