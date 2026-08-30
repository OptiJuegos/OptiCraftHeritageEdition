#include "ThreadSleepForever.h"

#include <chrono>

#include "Minecraft.h"

ThreadSleepForever::ThreadSleepForever(Minecraft *minecraft, const std::string &)
	: stopping(false)
	, worker(&ThreadSleepForever::run, this)
{
	(void)minecraft;
}

ThreadSleepForever::~ThreadSleepForever()
{
	stopping.store(true);
	stopCondition.notify_all();
	if (worker.joinable() && worker.get_id() != std::this_thread::get_id())
		worker.join();
}

void ThreadSleepForever::run()
{
	std::unique_lock<std::mutex> lock(stopMutex);
	while (!stopping.load())
	{
		stopCondition.wait_for(lock, std::chrono::milliseconds(0x7fffffff), [this]()
		{
			return stopping.load();
		});
	}
}
