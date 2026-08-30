#pragma once

#include <map>
#include <mutex>
#include <string>

#include "java/Type.h"

class PlayerUsageSnooper
{
public:
    explicit PlayerUsageSnooper(const std::string &endpointName);

    void func_52022_a(const std::string &key, const std::string &value);
    void func_52022_a(const std::string &key, const char *value);
    void func_52022_a(const std::string &key, int_t value);
    void func_52022_a(const std::string &key, long_t value);
    void func_52021_a();

    const std::string &getUrl() const;
    std::map<std::string, std::string> snapshotParameters() const;

private:
    mutable std::mutex mutex;
    std::map<std::string, std::string> parameters;
    std::string url;
};
