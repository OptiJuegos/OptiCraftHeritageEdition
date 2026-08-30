#pragma once

#include <thread>

class NetworkManager;

// net.minecraft.src.NetworkMasterThread
class NetworkMasterThread
{
public:
    NetworkMasterThread(NetworkManager *networkmanager);

    void start();
    void run();

    NetworkManager *netManager;

private:
    std::thread thread;
};
