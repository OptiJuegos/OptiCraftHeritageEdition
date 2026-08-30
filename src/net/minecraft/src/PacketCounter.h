#pragma once

#include "java/Type.h"

// net.minecraft.src.PacketCounter
class PacketCounter
{
public:
	PacketCounter();
	void addPacket(int_t size);

private:
	int_t totalPackets = 0;
	long_t totalBytes = 0;
};
