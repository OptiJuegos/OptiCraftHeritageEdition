#include "platform/Log.h"
#include "ThreadStatSyncherSend.h"

#include <iostream>

#include "StatsSyncher.h"

ThreadStatSyncherSend::ThreadStatSyncherSend(StatsSyncher *statssyncher, std::map<StatBase*, int_t> map)
	: field_27233_a(map)
	, field_27232_b(statssyncher)
{
}

ThreadStatSyncherSend::~ThreadStatSyncherSend()
{
	if (worker.joinable())
		worker.detach();
}

void ThreadStatSyncherSend::start()
{
	worker = std::thread(&ThreadStatSyncherSend::run, this);
	worker.detach();
}

void ThreadStatSyncherSend::run()
{
	try
	{
		field_27232_b->sendStats(field_27233_a);
	}
	catch (std::exception &exception)
	{
		MC_LOG_ERROR("game", "%s\n", exception.what());
		field_27232_b->setBusy(false);
	}
}
