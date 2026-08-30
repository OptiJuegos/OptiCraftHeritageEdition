#include "platform/Log.h"
#include "NetworkMasterThread.h"

#include <chrono>
#include <iostream>

#include "NetworkManager.h"

NetworkMasterThread::NetworkMasterThread(NetworkManager *networkmanager)
    : netManager(networkmanager)
{
}

void NetworkMasterThread::start()
{
    thread = std::thread(&NetworkMasterThread::run, this);
    thread.detach();
}

void NetworkMasterThread::run()
{
    try
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));

        // Java: if (readThread.isAlive()) readThread.stop()
        // C++ equivalent: detach so it no longer blocks join
        std::thread *readThread = NetworkManager::getReadThread(netManager);
        if (readThread != nullptr && readThread->joinable())
        {
            try { readThread->detach(); }
            catch (...) {}
        }

        std::thread *writeThread = NetworkManager::getWriteThread(netManager);
        if (writeThread != nullptr && writeThread->joinable())
        {
            try { writeThread->detach(); }
            catch (...) {}
        }
    }
    catch (...)
    {
        MC_LOG_WARN("network", "NetworkMasterThread: interrupted\n");
    }
}
