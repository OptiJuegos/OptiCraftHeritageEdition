#include "platform/Log.h"
#include "NetworkWriterThread.h"

#include <chrono>
#include <iostream>
#include <ostream>

#include "NetworkManager.h"

NetworkWriterThread::NetworkWriterThread(NetworkManager *networkmanager, const std::string &name)
    : netManager(networkmanager), threadName(name)
{
}

void NetworkWriterThread::start()
{
    alive = true;
    thread = std::thread(&NetworkWriterThread::run, this);
}

bool NetworkWriterThread::isAlive() const
{
    return alive.load();
}

void NetworkWriterThread::join()
{
    if (thread.joinable())
        thread.join();
}

void NetworkWriterThread::run()
{
    {
        std::lock_guard<std::mutex> lock(NetworkManager::threadSyncObject);
        NetworkManager::numWriteThreads++;
    }

    try
    {
        while (NetworkManager::isRunning(netManager))
        {
            while (NetworkManager::sendNetworkPacket(netManager)) ;

            try
            {
                std::ostream *os = NetworkManager::getSocketOutputStream(netManager);
                if (os != nullptr)
                    os->flush();
            }
            catch (std::exception &ioexception)
            {
                if (!NetworkManager::isTerminating(netManager))
                {
                    NetworkManager::handleNetworkException(netManager, ioexception);
                }
                MC_LOG_ERROR("game", "%s\n", ioexception.what());
            }

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
        NetworkManager::numWriteThreads--;
    }

    alive = false;
}
