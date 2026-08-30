#pragma once

#include <thread>

class StatsSyncher;

// net.minecraft.src.ThreadStatSyncherReceive
class ThreadStatSyncherReceive
{
public:
	ThreadStatSyncherReceive(StatsSyncher *statssyncher);
	~ThreadStatSyncherReceive();

	void start();
	void run();

private:
	StatsSyncher *field_27231_a;
	std::thread worker;
};
