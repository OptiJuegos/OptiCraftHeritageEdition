#include "PlayerUsageSnooper.h"

#include "PlayerUsageSnooperThread.h"

PlayerUsageSnooper::PlayerUsageSnooper(const std::string &endpointName)
    : url("http://snoop.minecraft.net/" + endpointName)
{
}

void PlayerUsageSnooper::func_52022_a(const std::string &key, const std::string &value)
{
    std::lock_guard<std::mutex> guard(mutex);
    parameters[key] = value;
}

void PlayerUsageSnooper::func_52022_a(const std::string &key, const char *value)
{
    func_52022_a(key, value != nullptr ? std::string(value) : std::string());
}

void PlayerUsageSnooper::func_52022_a(const std::string &key, int_t value)
{
    func_52022_a(key, std::to_string(value));
}

void PlayerUsageSnooper::func_52022_a(const std::string &key, long_t value)
{
    func_52022_a(key, std::to_string(value));
}

void PlayerUsageSnooper::func_52021_a()
{
    PlayerUsageSnooperThread::startDetached(this);
}

const std::string &PlayerUsageSnooper::getUrl() const
{
    return url;
}

std::map<std::string, std::string> PlayerUsageSnooper::snapshotParameters() const
{
    std::lock_guard<std::mutex> guard(mutex);
    return parameters;
}
