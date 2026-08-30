#include "platform/Log.h"
#include "ThreadCloseConnection.h"

#include <chrono>
#include <iostream>
#include <vector>

#include "NetworkManager.h"

ThreadCloseConnection::ThreadCloseConnection(NetworkManager *networkmanager)
	: field_28109_a(networkmanager)
{
}

ThreadCloseConnection::~ThreadCloseConnection()
{
	if (worker.joinable())
		worker.detach();
}

void ThreadCloseConnection::start()
{
	worker = std::thread(&ThreadCloseConnection::run, this);
	worker.detach();
}

void ThreadCloseConnection::run()
{
	try
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(2000));
		if (NetworkManager::isRunning(field_28109_a))
			field_28109_a->networkShutdown("disconnect.closed", std::vector<std::string>());
	}
	catch (std::exception &exception)
	{
		MC_LOG_ERROR("game", "%s\n", exception.what());
	}
}
