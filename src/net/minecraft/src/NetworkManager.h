#pragma once

#include <deque>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <string>
#include <thread>
#include <vector>
#include <atomic>
#include <iosfwd>

#include "platform/Thread.h"

#include "java/Type.h"

namespace JavaNetwork
{
class Socket;
}

class NetHandler;
class Packet;

// net.minecraft.src.NetworkManager
class NetworkManager
{
public:
	NetworkManager(const std::string &host, int_t port, const std::string &s, NetHandler *nethandler);
	~NetworkManager();

	void addToSendQueue(Packet *packet);
	void processReadPackets();
	void wakeThreads();
	void networkShutdown(const std::string &s, const std::vector<std::string> &aobj);
	void serverShutdown() { closeConnection(); }
	void closeConnection();
	void flushQueue() { closeConnection(); }

	static bool isRunning(NetworkManager *networkmanager);
	static bool isServerTerminating(NetworkManager *networkmanager);
	static bool readNetworkPacket(NetworkManager *networkmanager);
	static bool sendNetworkPacket(NetworkManager *networkmanager);
	static bool isTerminating(NetworkManager *networkmanager);
	static void handleNetworkException(NetworkManager *networkmanager, std::exception &exception);
	static std::thread *getReadThread(NetworkManager *networkmanager);
	static std::thread *getWriteThread(NetworkManager *networkmanager);

	static std::mutex threadSyncObject;
	static std::ostream *getSocketOutputStream(NetworkManager *networkmanager);

	static int_t field_28145_d[256];
	static int_t field_28144_e[256];
	static std::atomic<int_t> numReadThreads;
	static std::atomic<int_t> numWriteThreads;

	int_t chunkDataSendCounter;

private:
	bool readPacket();
	bool sendPacket();
	void onNetworkError(std::exception &exception);
	void readThreadRun();
	void writeThreadRun();
	void sleepThread();
#ifdef WII_PLATFORM
	static void *wiiReadThreadEntry(void *argument);
	static void *wiiWriteThreadEntry(void *argument);
#endif

	std::mutex sendQueueLock;
	std::mutex readQueueLock;
	std::mutex shutdownLock;
	std::mutex threadSleepLock;
	std::condition_variable threadSleepCondition;
	std::unique_ptr<JavaNetwork::Socket> networkSocket;
	std::unique_ptr<std::istream> socketInputStream;
	std::unique_ptr<std::ostream> socketOutputStream;
	std::string remoteSocketAddress;
	std::atomic_bool running;
	std::atomic_bool serverTerminating;
	std::atomic_bool terminating;
	std::string terminationReason;
	std::vector<std::string> field_20101_t;
	std::deque<std::unique_ptr<Packet>> readPackets;
	std::deque<std::unique_ptr<Packet>> dataPackets;
	std::deque<std::unique_ptr<Packet>> chunkDataPackets;
	NetHandler *netHandler;
	bool serverHandler;
	std::thread readThread;
	std::thread writeThread;
#ifndef WII_PLATFORM
	std::thread closeThread;
#endif
#ifdef WII_PLATFORM
	PlatformThread wiiReadThread;
	PlatformThread wiiWriteThread;
#endif
	int_t timeSinceLastRead;
	int_t sendQueueByteLength;
	std::size_t readQueueByteLength;
	int_t field_20100_w;
};
