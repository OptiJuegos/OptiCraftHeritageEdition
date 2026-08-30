#pragma once

#include <thread>

class NetworkManager;

// net.minecraft.src.ThreadMonitorConnection
class ThreadMonitorConnection
{
public:
    explicit ThreadMonitorConnection(NetworkManager *networkManager);
    ~ThreadMonitorConnection();

    void start();
    void run();

private:
    NetworkManager *netManager;
    std::thread worker;
};
