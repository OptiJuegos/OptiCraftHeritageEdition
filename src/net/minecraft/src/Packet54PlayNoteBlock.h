#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet54PlayNoteBlock
class Packet54PlayNoteBlock : public Packet
{
public:
	Packet54PlayNoteBlock() = default;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t xLocation = 0;
	int_t yLocation = 0;
	int_t zLocation = 0;
	int_t instrumentType = 0;
	int_t pitch = 0;
};
