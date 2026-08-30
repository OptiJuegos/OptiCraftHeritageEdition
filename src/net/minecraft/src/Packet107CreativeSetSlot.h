#pragma once

#include "Packet.h"

class ItemStack;
class NetHandler;

// net.minecraft.src.Packet107CreativeSetSlot
class Packet107CreativeSetSlot : public Packet
{
public:
	Packet107CreativeSetSlot() = default;
	Packet107CreativeSetSlot(int_t slot, ItemStack *stack);
	~Packet107CreativeSetSlot() override;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t slot = 0;
	ItemStack *itemStack = nullptr;
};
