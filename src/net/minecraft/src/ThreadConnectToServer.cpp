#include "ThreadConnectToServer.h"

#include "platform/Log.h"
#ifdef PS2_PLATFORM
#include "ps2/system/Ps2ThreadPriority.h"
#endif
#include <exception>
#include <iostream>
#include <stdexcept>

#include "GuiConnecting.h"
#include "Minecraft.h"
#include "NetClientHandler.h"
#include "Packet2Handshake.h"
#include "Session.h"
#include "java/String.h"

ThreadConnectToServer::ThreadConnectToServer(GuiConnecting *guiconnecting, Minecraft *minecraft, const std::string &s, int_t i)
	: mc(minecraft)
	, hostName(s)
	, port(i)
{
	(void)guiconnecting;
}

ThreadConnectToServer::~ThreadConnectToServer()
{
	cancel();
	if (worker.joinable() && !worker.isCurrent())
		worker.join();
	std::lock_guard<PlatformMutex> guard(resultLock);
	if (resultHandler != nullptr)
	{
		resultHandler->disconnect();
		delete resultHandler;
		resultHandler = nullptr;
	}
}

void ThreadConnectToServer::start()
{
#ifdef PS2_PLATFORM
	constexpr int kConnectPriority = Ps2ThreadPriority::kNetwork;
#else
	constexpr int kConnectPriority = 64;
#endif
	if (!worker.start(&ThreadConnectToServer::wiiThreadEntry, this, 32 * 1024, kConnectPriority))
		throw std::runtime_error("Could not create connection thread");
}

void ThreadConnectToServer::cancel()
{
	cancelled.store(true);
}

NetClientHandler *ThreadConnectToServer::takeHandler()
{
	std::lock_guard<PlatformMutex> guard(resultLock);
	NetClientHandler *handler = resultHandler;
	resultHandler = nullptr;
	return handler;
}

bool ThreadConnectToServer::takeError(std::string &message)
{
	std::lock_guard<PlatformMutex> guard(resultLock);
	if (!errorPending)
		return false;
	message = resultError;
	errorPending = false;
	return true;
}

void *ThreadConnectToServer::wiiThreadEntry(void *argument)
{
	static_cast<ThreadConnectToServer *>(argument)->run();
	return nullptr;
}

void ThreadConnectToServer::run()
{
	try
	{
		MC_LOG_INFO("network", "[PS2] connect worker running for %s:%d\n", hostName.c_str(), static_cast<int>(port));
		McLog::flush();
		NetClientHandler *handler = new NetClientHandler(mc, hostName, port);
		if (cancelled.load())
		{
			handler->disconnect();
			delete handler;
			return;
		}
		handler->addToSendQueue(new Packet2Handshake(mc->session->username));
		std::lock_guard<PlatformMutex> guard(resultLock);
		resultHandler = handler;
	}
	catch (std::exception &exception)
	{
		if (cancelled.load())
			return;
		MC_LOG_ERROR("game", "%s\n", exception.what());
		std::lock_guard<PlatformMutex> guard(resultLock);
		resultError = exception.what();
		errorPending = true;
	}
}
