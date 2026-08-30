#include "ThreadClientSleep.h"

#include <chrono>

#include "Minecraft.h"

ThreadClientSleep::ThreadClientSleep(Minecraft *minecraft, const char *)
    : mc(minecraft), stopping(false), worker(&ThreadClientSleep::run, this)
{
}

ThreadClientSleep::~ThreadClientSleep()
{
    stopping.store(true);
    condition.notify_all();
    if (worker.joinable() && worker.get_id() != std::this_thread::get_id())
        worker.join();
}

void ThreadClientSleep::run()
{
    std::unique_lock<std::mutex> lock(mutex);
    while (!stopping.load() && mc != nullptr && mc->running)
    {
        condition.wait_for(lock, std::chrono::milliseconds(0x7fffffff), [this]()
        {
            return stopping.load() || mc == nullptr || !mc->running;
        });
    }
}
