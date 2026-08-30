#include "NetworkReaderThread.h"

#include <chrono>

#include "NetworkManager.h"

NetworkReaderThread::NetworkReaderThread(NetworkManager *networkmanager, const std::string &name)
    : netManager(networkmanager), threadName(name)
{
}

void NetworkReaderThread::start()
{
    alive = true;
    thread = std::thread(&NetworkReaderThread::run, this);
}

bool NetworkReaderThread::isAlive() const
{
    return alive.load();
}

void NetworkReaderThread::join()
{
    if (thread.joinable())
        thread.join();
}

void NetworkReaderThread::run()
{
    {
        std::lock_guard<std::mutex> lock(NetworkManager::threadSyncObject);
        NetworkManager::numReadThreads++;
    }

    try
    {
        while (NetworkManager::isRunning(netManager) && !NetworkManager::isServerTerminating(netManager))
        {
            while (NetworkManager::readNetworkPacket(netManager)) ;
            try
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }
            catch (...) {}
        }
    }
    catch (...)
    {
        // absorb
    }

    {
        std::lock_guard<std::mutex> lock(NetworkManager::threadSyncObject);
        NetworkManager::numReadThreads--;
    }

    alive = false;
}
