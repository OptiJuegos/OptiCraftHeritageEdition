#include "PostHttp.h"

#include <cctype>
#include <cstdio>
#include <vector>

#include "java/JavaNetwork.h"
#include "platform/Log.h"

std::string PostHttp::urlEncode(const std::string &value)
{
    static const char hex[] = "0123456789ABCDEF";
    std::string encoded;
    encoded.reserve(value.size() * 3);
    for (unsigned char ch : value)
    {
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
            (ch >= '0' && ch <= '9') || ch == '-' || ch == '_' || ch == '.' || ch == '*')
        {
            encoded.push_back((char)ch);
        }
        else if (ch == ' ')
        {
            encoded.push_back('+');
        }
        else
        {
            encoded.push_back('%');
            encoded.push_back(hex[(ch >> 4) & 0xf]);
            encoded.push_back(hex[ch & 0xf]);
        }
    }
    return encoded;
}

std::string PostHttp::func_52016_a(const Parameters &parameters)
{
    std::string body;
    for (const auto &entry : parameters)
    {
        if (!body.empty())
            body.push_back('&');
        body += urlEncode(entry.first);
        body.push_back('=');
        body += urlEncode(entry.second);
    }
    return body;
}

std::string PostHttp::func_52018_a(const std::string &url, const Parameters &parameters, bool silent)
{
    return func_52017_a(url, func_52016_a(parameters), silent);
}

std::string PostHttp::func_52017_a(const std::string &url, const std::string &body, bool silent)
{
    std::vector<unsigned char> response;
    if (!JavaNetwork::postUrl(url, "application/x-www-form-urlencoded", body, response))
    {
        if (!silent)
            MC_LOG_ERROR("network", "Could not post to %s\n", url.c_str());
        return std::string();
    }
    return std::string(reinterpret_cast<const char *>(response.data()), response.size());
}
