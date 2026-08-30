#pragma once

#include <memory>

class ServerNBTStorage;

// net.minecraft.src.ThreadPollServers
class ThreadPollServers
{
public:
    static void start(const std::shared_ptr<ServerNBTStorage> &server);

private:
    static void run(std::shared_ptr<ServerNBTStorage> server);
};
