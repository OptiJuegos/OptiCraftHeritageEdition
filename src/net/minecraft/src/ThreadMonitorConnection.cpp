#include "ThreadMonitorConnection.h"

#include <chrono>
#include <vector>

#include "NetworkManager.h"
#include "platform/Log.h"

ThreadMonitorConnection::ThreadMonitorConnection(NetworkManager *networkManager)
    : netManager(networkManager)
{
}

ThreadMonitorConnection::~ThreadMonitorConnection()
{
    if (worker.joinable() && worker.get_id() != std::this_thread::get_id())
        worker.join();
}

void ThreadMonitorConnection::start()
{
    if (!worker.joinable())
        worker = std::thread(&ThreadMonitorConnection::run, this);
}

void ThreadMonitorConnection::run()
{
    try
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        if (netManager != nullptr && NetworkManager::isRunning(netManager))
        {
            std::thread *writeThread = NetworkManager::getWriteThread(netManager);
            (void)writeThread; // C++ shutdown wakes the owned writer instead of interrupting std::thread.
            netManager->networkShutdown("disconnect.closed", std::vector<std::string>());
        }
    }
    catch (const std::exception &exception)
    {
        MC_LOG_ERROR("network", "ThreadMonitorConnection: %s\n", exception.what());
    }
}
