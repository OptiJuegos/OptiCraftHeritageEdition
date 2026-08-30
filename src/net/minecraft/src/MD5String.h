#pragma once

#include <string>

// net.minecraft.src.MD5String
class MD5String
{
public:
    MD5String(const std::string &s);

    std::string computeHash(const std::string &s);

private:
    std::string prefix;

    static std::string md5Hex(const std::string &input);
};
