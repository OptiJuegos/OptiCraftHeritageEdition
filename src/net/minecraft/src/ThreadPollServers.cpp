#include "ThreadPollServers.h"

#include <chrono>
#include <thread>

#include "GuiMultiplayer.h"
#include "ServerNBTStorage.h"

void ThreadPollServers::start(const std::shared_ptr<ServerNBTStorage> &server)
{
#if defined(PS2_PLATFORM) || defined(WII_PLATFORM)
    run(server);
#else
    std::thread(&ThreadPollServers::run, server).detach();
#endif
}

void ThreadPollServers::run(std::shared_ptr<ServerNBTStorage> server)
{
    if (server == nullptr)
    {
        GuiMultiplayer::decrementThreadsPending();
        return;
    }

    try
    {
        const auto startTime = std::chrono::steady_clock::now();
        GuiMultiplayer::pollServer(server);
        const auto endTime = std::chrono::steady_clock::now();
        const long_t latency = (long_t)std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
        std::lock_guard<std::mutex> guard(server->stateMutex);
        server->lag = latency;
    }
    catch (...)
    {
        std::lock_guard<std::mutex> guard(server->stateMutex);
        server->lag = -1;
        server->motd = "\xC2\xA7" "4Can't reach server";
        server->playerCount.clear();
    }

    GuiMultiplayer::decrementThreadsPending();
}
