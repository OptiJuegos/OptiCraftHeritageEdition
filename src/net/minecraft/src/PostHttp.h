#pragma once

#include <map>
#include <string>

// net.minecraft.src.PostHttp
class PostHttp
{
public:
    using Parameters = std::map<std::string, std::string>;

    static std::string func_52016_a(const Parameters &parameters);
    static std::string func_52018_a(const std::string &url, const Parameters &parameters, bool silent);
    static std::string func_52017_a(const std::string &url, const std::string &body, bool silent);

private:
    static std::string urlEncode(const std::string &value);
};
