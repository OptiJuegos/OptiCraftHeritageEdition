#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include "platform/Thread.h"

#include "java/Type.h"

class GuiConnecting;
class Minecraft;
class NetClientHandler;

// net.minecraft.src.ThreadConnectToServer
class ThreadConnectToServer
{
public:
	ThreadConnectToServer(GuiConnecting *guiconnecting, Minecraft *minecraft, const std::string &s, int_t i);
	~ThreadConnectToServer();

	void start();
	void run();
	void cancel();
	NetClientHandler *takeHandler();
	bool takeError(std::string &message);

private:
	static void *wiiThreadEntry(void *argument);
	Minecraft *mc;
	std::string hostName;
	int_t port;
	std::mutex resultLock;
	NetClientHandler *resultHandler = nullptr;
	std::string resultError;
	bool errorPending = false;
	std::atomic_bool cancelled{false};
	PlatformThread worker;
};
