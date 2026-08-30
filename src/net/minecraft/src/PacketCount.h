#pragma once

#include <mutex>
#include <unordered_map>

#include "java/Type.h"

// net.minecraft.src.PacketCount
class PacketCount
{
public:
	static void countPacket(int_t packetId, long_t size);
	static long_t getPacketCount(int_t packetId);
	static long_t getSizeCount(int_t packetId);

	static bool allowCounting;

private:
	static std::unordered_map<int_t, long_t> packetCountForID;
	static std::unordered_map<int_t, long_t> sizeCountForID;
	static std::mutex lock;
};
