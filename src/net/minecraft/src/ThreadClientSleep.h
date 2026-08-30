#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

class Minecraft;

// net.minecraft.src.ThreadClientSleep
class ThreadClientSleep
{
public:
    ThreadClientSleep(Minecraft *minecraft, const char *name);
    ~ThreadClientSleep();

    void run();

private:
    Minecraft *mc;
    std::atomic_bool stopping;
    std::mutex mutex;
    std::condition_variable condition;
    std::thread worker;
};
