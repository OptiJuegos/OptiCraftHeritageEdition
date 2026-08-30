#pragma once

#include "Packet.h"

class NetHandler;
class EntityPainting;

// net.minecraft.src.Packet25EntityPainting
class Packet25EntityPainting : public Packet
{
public:
	Packet25EntityPainting() = default;
	Packet25EntityPainting(EntityPainting *entitypainting);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t entityId = 0;
	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t direction = 0;
	jstring title;
};
