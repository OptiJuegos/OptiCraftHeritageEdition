#include "platform/Log.h"
#include "ThreadStatSyncherReceive.h"

#include <iostream>

#include "StatsSyncher.h"

ThreadStatSyncherReceive::ThreadStatSyncherReceive(StatsSyncher *statssyncher)
	: field_27231_a(statssyncher)
{
}

ThreadStatSyncherReceive::~ThreadStatSyncherReceive()
{
	if (worker.joinable())
		worker.detach();
}

void ThreadStatSyncherReceive::start()
{
	worker = std::thread(&ThreadStatSyncherReceive::run, this);
	worker.detach();
}

void ThreadStatSyncherReceive::run()
{
	try
	{
		field_27231_a->receiveStats();
	}
	catch (std::exception &exception)
	{
		MC_LOG_ERROR("game", "%s\n", exception.what());
		field_27231_a->setBusy(false);
	}
}
