#pragma once

#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

class Minecraft;

// net.minecraft.src.ThreadSleepForever
class ThreadSleepForever
{
public:
	ThreadSleepForever(Minecraft *minecraft, const std::string &s);
	~ThreadSleepForever();

	void run();

private:
	std::atomic_bool stopping;
	std::mutex stopMutex;
	std::condition_variable stopCondition;
	std::thread worker;
};
