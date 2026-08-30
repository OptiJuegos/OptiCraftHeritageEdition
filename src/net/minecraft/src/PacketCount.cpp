#include "PacketCount.h"

bool PacketCount::allowCounting = true;
std::unordered_map<int_t, long_t> PacketCount::packetCountForID;
std::unordered_map<int_t, long_t> PacketCount::sizeCountForID;
std::mutex PacketCount::lock;

void PacketCount::countPacket(int_t packetId, long_t size)
{
	if (!allowCounting)
		return;

	std::lock_guard<std::mutex> guard(lock);
	++packetCountForID[packetId];
	sizeCountForID[packetId] += size;
}

long_t PacketCount::getPacketCount(int_t packetId)
{
	std::lock_guard<std::mutex> guard(lock);
	auto it = packetCountForID.find(packetId);
	return it == packetCountForID.end() ? 0 : it->second;
}

long_t PacketCount::getSizeCount(int_t packetId)
{
	std::lock_guard<std::mutex> guard(lock);
	auto it = sizeCountForID.find(packetId);
	return it == sizeCountForID.end() ? 0 : it->second;
}
