#pragma once

#include <string>
#include <thread>
#include <atomic>

class NetworkManager;

// net.minecraft.src.NetworkWriterThread
class NetworkWriterThread
{
public:
    NetworkWriterThread(NetworkManager *networkmanager, const std::string &name);

    void start();
    bool isAlive() const;
    void join();
    void run();

    NetworkManager *netManager;

private:
    std::string threadName;
    std::thread thread;
    std::atomic_bool alive{false};
};
