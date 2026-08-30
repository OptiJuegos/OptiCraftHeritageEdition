#include "PacketCounter.h"

PacketCounter::PacketCounter() :
	totalPackets(0),
	totalBytes(0)
{
}

void PacketCounter::addPacket(int_t size)
{
	totalPackets++;
	totalBytes += size;
}
