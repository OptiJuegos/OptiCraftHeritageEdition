#pragma once

#include "Packet.h"

class NetHandler;
class Entity;

// net.minecraft.src.Packet71Weather
class Packet71Weather : public Packet
{
public:
	Packet71Weather() = default;
	Packet71Weather(Entity *entity);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;       // nbtTagType
	int_t weatherType = 0;    // nbtTagName
	int_t posX = 0;           // nbtTagList
	int_t posY = 0;           // nbtTagCompound
	int_t posZ = 0;           // nbtTagValue
};
