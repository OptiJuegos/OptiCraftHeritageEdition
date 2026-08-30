#include "platform/Log.h"
#include "NetworkManager.h"

#include <chrono>
#include <iostream>
#include <stdexcept>

#ifdef WII_PLATFORM
#include <unistd.h>
#endif

#include "NetHandler.h"
#include "Packet.h"
#include "java/JavaNetwork.h"
#include "java/Arithmetic.h"
#include "java/System.h"

int_t NetworkManager::field_28145_d[256];
int_t NetworkManager::field_28144_e[256];
std::atomic<int_t> NetworkManager::numReadThreads{0};
std::atomic<int_t> NetworkManager::numWriteThreads{0};
std::mutex NetworkManager::threadSyncObject;

NetworkManager::NetworkManager(const std::string &host, int_t port, const std::string &s, NetHandler *nethandler)
	: networkSocket(JavaNetwork::createSocket())
	, running(true)
	, serverTerminating(false)
	, terminating(false)
	, terminationReason("")
	, netHandler(nethandler)
	, serverHandler(nethandler != nullptr && nethandler->isServerHandler())
	, chunkDataSendCounter(0)
	, timeSinceLastRead(0)
	, sendQueueByteLength(0)
	, readQueueByteLength(0)
	, field_20100_w(50)
{
	if (networkSocket == nullptr || !networkSocket->connect(host, port))
		throw std::runtime_error("Connection refused: " + host + ":" + std::to_string(port));

	remoteSocketAddress = networkSocket->getRemoteSocketAddress();
	socketInputStream = JavaNetwork::createInputStream(*networkSocket);
	socketOutputStream = JavaNetwork::createOutputStream(*networkSocket);
	if (socketInputStream == nullptr || socketOutputStream == nullptr)
		throw std::runtime_error("Could not create network streams");
	socketOutputStream->exceptions(std::ios::badbit | std::ios::failbit);
#ifdef WII_PLATFORM
	if (!wiiReadThread.start(&NetworkManager::wiiReadThreadEntry, this, 32 * 1024, 64))
	{
		networkSocket->close();
		throw std::runtime_error("Could not create Wii network read thread");
	}
	if (!wiiWriteThread.start(&NetworkManager::wiiWriteThreadEntry, this, 32 * 1024, 64))
	{
		running = false;
		networkSocket->close();
		wiiReadThread.join();
		throw std::runtime_error("Could not create Wii network write thread");
	}
#else
	try
	{
		readThread = std::thread(&NetworkManager::readThreadRun, this);
		writeThread = std::thread(&NetworkManager::writeThreadRun, this);
	}
	catch (...)
	{
		running = false;
		if (networkSocket != nullptr)
			networkSocket->close();
		wakeThreads();
		if (readThread.joinable())
			readThread.join();
		throw;
	}
#endif
	(void)s;
}

NetworkManager::~NetworkManager()
{
	networkShutdown("disconnect.closed", std::vector<std::string>());
#ifdef WII_PLATFORM
	if (wiiReadThread.joinable() && !wiiReadThread.isCurrent()) wiiReadThread.join();
	if (wiiWriteThread.joinable() && !wiiWriteThread.isCurrent()) wiiWriteThread.join();
#else
	if (closeThread.joinable() && closeThread.get_id() != std::this_thread::get_id())
		closeThread.join();
	if (readThread.joinable() && readThread.get_id() != std::this_thread::get_id())
		readThread.join();
	if (writeThread.joinable() && writeThread.get_id() != std::this_thread::get_id())
		writeThread.join();
#endif
}

void NetworkManager::addToSendQueue(Packet *packet)
{
	std::unique_ptr<Packet> ownedPacket(packet);
	if (ownedPacket == nullptr || serverTerminating || terminating || !running)
		return;

	std::lock_guard<std::mutex> guard(sendQueueLock);
	sendQueueByteLength += ownedPacket->getPacketSize() + 1;
	if (ownedPacket->isChunkDataPacket)
		chunkDataPackets.emplace_back(std::move(ownedPacket));
	else
		dataPackets.emplace_back(std::move(ownedPacket));
	wakeThreads();
}

bool NetworkManager::sendPacket()
{
	bool flag = false;
	try
	{
		if (socketOutputStream == nullptr)
			return false;

		std::unique_ptr<Packet> packet;
		{
			std::lock_guard<std::mutex> guard(sendQueueLock);
			if (!dataPackets.empty() && (chunkDataSendCounter == 0 || JavaArithmetic::longSub(System::currentTimeMillis(), dataPackets[0]->creationTimeMillis) >= chunkDataSendCounter))
			{
				packet = std::move(dataPackets.front());
				dataPackets.pop_front();
				sendQueueByteLength -= packet->getPacketSize() + 1;
			}
		}
		if (packet != nullptr)
		{
			Packet::writePacket(packet.get(), *socketOutputStream);
			if (!socketOutputStream->good())
				throw std::runtime_error("Failed to write network packet");
			field_28144_e[packet->getPacketId()] += packet->getPacketSize() + 1;
			flag = true;
		}

		std::unique_ptr<Packet> packet1;
		{
			std::lock_guard<std::mutex> guard(sendQueueLock);
			if (field_20100_w-- <= 0 && !chunkDataPackets.empty() && (chunkDataSendCounter == 0 || JavaArithmetic::longSub(System::currentTimeMillis(), chunkDataPackets[0]->creationTimeMillis) >= chunkDataSendCounter))
			{
				packet1 = std::move(chunkDataPackets.front());
				chunkDataPackets.pop_front();
				sendQueueByteLength -= packet1->getPacketSize() + 1;
			}
		}
		if (packet1 != nullptr)
		{
			Packet::writePacket(packet1.get(), *socketOutputStream);
			if (!socketOutputStream->good())
				throw std::runtime_error("Failed to write chunk packet");
			field_28144_e[packet1->getPacketId()] += packet1->getPacketSize() + 1;
			field_20100_w = 0;
			flag = true;
		}
	}
	catch (std::exception &exception)
	{
		if (!terminating)
			onNetworkError(exception);
		return false;
	}
	return flag;
}

void NetworkManager::wakeThreads()
{
	threadSleepCondition.notify_all();
}

bool NetworkManager::readPacket()
{
	#ifdef WII_PLATFORM
	constexpr std::size_t MAX_READ_QUEUE_BYTES = 4 * 1024 * 1024;
	constexpr std::size_t MAX_READ_QUEUE_PACKETS = 2048;
	#else
	constexpr std::size_t MAX_READ_QUEUE_BYTES = 32 * 1024 * 1024;
	constexpr std::size_t MAX_READ_QUEUE_PACKETS = 8192;
	#endif
	bool flag = false;
	try
	{
		if (socketInputStream == nullptr)
			return false;

		std::unique_ptr<Packet> packet = Packet::readPacket(*socketInputStream, serverHandler);
		if (packet != nullptr)
		{
			const int_t packetBytesSigned = packet->getPacketSize() + 1;
			if (packetBytesSigned <= 0)
				throw std::runtime_error("Invalid incoming packet size");
			const std::size_t packetBytes = static_cast<std::size_t>(packetBytesSigned);
			field_28145_d[packet->getPacketId()] += packetBytesSigned;
			std::lock_guard<std::mutex> guard(readQueueLock);
			if (readPackets.size() >= MAX_READ_QUEUE_PACKETS ||
			    readQueueByteLength > MAX_READ_QUEUE_BYTES ||
			    packetBytes > MAX_READ_QUEUE_BYTES - readQueueByteLength)
				throw std::runtime_error("Incoming packet queue overflow");
			readQueueByteLength += packetBytes;
			readPackets.emplace_back(std::move(packet));
			flag = true;
		}
		else if (!serverTerminating)
		{
			networkShutdown("disconnect.endOfStream", std::vector<std::string>());
		}
	}
	catch (std::exception &exception)
	{
		if (!terminating)
			onNetworkError(exception);
		return false;
	}
	return flag;
}

void NetworkManager::onNetworkError(std::exception &exception)
{
	MC_LOG_ERROR("game", "%s\n", exception.what());
	networkShutdown("disconnect.genericReason", std::vector<std::string>{std::string("Internal exception: ") + exception.what()});
}

void NetworkManager::networkShutdown(const std::string &s, const std::vector<std::string> &aobj)
{
	std::lock_guard<std::mutex> shutdownGuard(shutdownLock);
	if (!running)
		return;

	terminationReason = s;
	field_20101_t = aobj;
	terminating = true;
	running = false;
	wakeThreads();

	// Only close the socket here so a blocked recv() in the read thread returns
	// and both threads observe running == false and exit. Do NOT free the stream
	// or socket objects: another thread may still be inside is.get()/flush().
	// They are released in the destructor, after both threads have been joined.
	if (networkSocket != nullptr)
		networkSocket->close();
}

void NetworkManager::processReadPackets()
{
	bool sendQueueOverflow;
	{
		std::lock_guard<std::mutex> guard(sendQueueLock);
		sendQueueOverflow = sendQueueByteLength > 0x100000;
	}
	if (sendQueueOverflow)
		networkShutdown("disconnect.overflow", std::vector<std::string>());

	bool empty;
	{
		std::lock_guard<std::mutex> guard(readQueueLock);
		empty = readPackets.empty();
	}

	if (empty)
	{
		if (timeSinceLastRead++ == 1200)
			networkShutdown("disconnect.timeout", std::vector<std::string>());
	}
	else
	{
		timeSinceLastRead = 0;
	}

	for (int_t i = 1000; i-- >= 0;)
	{
		std::unique_ptr<Packet> packet;
		{
			std::lock_guard<std::mutex> guard(readQueueLock);
			if (readPackets.empty())
				break;
			packet = std::move(readPackets.front());
			readPackets.pop_front();
			const int_t packetBytes = packet != nullptr ? packet->getPacketSize() + 1 : 0;
			if (packetBytes > 0 && static_cast<std::size_t>(packetBytes) <= readQueueByteLength)
				readQueueByteLength -= static_cast<std::size_t>(packetBytes);
			else if (packetBytes > 0)
				readQueueByteLength = 0;
		}
		if (packet != nullptr && netHandler != nullptr)
		{
			try
			{
				packet->processPacket(*netHandler);
			}
			catch (std::exception &exception)
			{
				onNetworkError(exception);
				std::lock_guard<std::mutex> guard(readQueueLock);
				readPackets.clear();
				readQueueByteLength = 0;
				break;
			}
			catch (...)
			{
				std::runtime_error exception("Unhandled exception while processing network packet");
				onNetworkError(exception);
				std::lock_guard<std::mutex> guard(readQueueLock);
				readPackets.clear();
				readQueueByteLength = 0;
				break;
			}
		}
	}

	wakeThreads();

	{
		std::lock_guard<std::mutex> guard(readQueueLock);
		empty = readPackets.empty();
	}
	if (terminating && empty && netHandler != nullptr)
		netHandler->handleErrorMessage(terminationReason, field_20101_t);
}

void NetworkManager::closeConnection()
{
	wakeThreads();
	serverTerminating = true;
	if (networkSocket != nullptr)
		networkSocket->interruptRead();

#ifdef WII_PLATFORM
	// The writer closes the connection after the queued disconnect packet has
	// been flushed. interruptRead() only shuts down the receive side on Wii.
#else
	// Java can detach this helper safely because the NetworkManager remains GC-reachable.
	// In C++, keep the delayed closer owned by the manager so it cannot outlive `this`.
	if (!closeThread.joinable())
	{
		closeThread = std::thread([this]()
		{
			std::unique_lock<std::mutex> lock(threadSleepLock);
			threadSleepCondition.wait_for(lock, std::chrono::milliseconds(2000), [this]()
			{
				return !running.load();
			});
			lock.unlock();
			if (running.load())
				networkShutdown("disconnect.closed", std::vector<std::string>());
		});
	}
#endif
}

#ifdef WII_PLATFORM
void *NetworkManager::wiiReadThreadEntry(void *argument)
{
	try { static_cast<NetworkManager *>(argument)->readThreadRun(); }
	catch (...) { /* Never unwind a C++ exception through the LWP C entry point. */ }
	return nullptr;
}

void *NetworkManager::wiiWriteThreadEntry(void *argument)
{
	try { static_cast<NetworkManager *>(argument)->writeThreadRun(); }
	catch (...) { /* Never unwind a C++ exception through the LWP C entry point. */ }
	return nullptr;
}
#endif

void NetworkManager::readThreadRun()
{
	numReadThreads++;
	try
	{
		while (running && !serverTerminating)
		{
			while (running && !serverTerminating && readPacket())
			{
			}
			sleepThread();
		}
	}
	catch (...)
	{
		numReadThreads--;
		throw;
	}
	numReadThreads--;
}

void NetworkManager::writeThreadRun()
{
	numWriteThreads++;
	try
	{
		while (running)
		{
			while (running && sendPacket())
			{
			}
			try
			{
				if (socketOutputStream != nullptr)
					socketOutputStream->flush();
			}
			catch (std::exception &exception)
			{
				if (!terminating)
					onNetworkError(exception);
				MC_LOG_ERROR("game", "%s\n", exception.what());
			}
			sleepThread();

			if (serverTerminating && running)
			{
				bool queueEmpty;
				{
					std::lock_guard<std::mutex> guard(sendQueueLock);
					queueEmpty = dataPackets.empty() && chunkDataPackets.empty();
				}
				if (queueEmpty)
					networkShutdown("disconnect.closed", std::vector<std::string>());
			}
		}
	}
	catch (...)
	{
		numWriteThreads--;
		throw;
	}
	numWriteThreads--;
}

void NetworkManager::sleepThread()
{
#ifdef WII_PLATFORM
	// A bounded sleep keeps shutdown latency low. The desktop condition variable
	// is intentionally avoided: its wait backend is not implemented by the Wii
	// libstdc++ build, for the same reason std::thread throws ENOSYS.
	usleep(2000);
#else
	std::unique_lock<std::mutex> lock(threadSleepLock);
	threadSleepCondition.wait_for(lock, std::chrono::milliseconds(2));
#endif
}

bool NetworkManager::isRunning(NetworkManager *networkmanager)
{
	return networkmanager != nullptr && networkmanager->running;
}

bool NetworkManager::isServerTerminating(NetworkManager *networkmanager)
{
	return networkmanager != nullptr && networkmanager->serverTerminating;
}

bool NetworkManager::readNetworkPacket(NetworkManager *networkmanager)
{
	return networkmanager != nullptr && networkmanager->readPacket();
}

bool NetworkManager::sendNetworkPacket(NetworkManager *networkmanager)
{
	return networkmanager != nullptr && networkmanager->sendPacket();
}

bool NetworkManager::isTerminating(NetworkManager *networkmanager)
{
	return networkmanager != nullptr && networkmanager->terminating;
}

void NetworkManager::handleNetworkException(NetworkManager *networkmanager, std::exception &exception)
{
	if (networkmanager != nullptr)
		networkmanager->onNetworkError(exception);
}

std::thread *NetworkManager::getReadThread(NetworkManager *networkmanager)
{
	return networkmanager != nullptr ? &networkmanager->readThread : nullptr;
}

std::thread *NetworkManager::getWriteThread(NetworkManager *networkmanager)
{
	return networkmanager != nullptr ? &networkmanager->writeThread : nullptr;
}

std::ostream *NetworkManager::getSocketOutputStream(NetworkManager *networkmanager)
{
	return networkmanager != nullptr ? networkmanager->socketOutputStream.get() : nullptr;
}
