#pragma once

#include "Packet.h"

class NetHandler;
class ItemStack;

// net.minecraft.src.Packet103SetSlot
class Packet103SetSlot : public Packet
{
public:
	Packet103SetSlot() = default;
	~Packet103SetSlot() override;

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;
	ItemStack *releaseItemStack();

	int_t windowId = 0;
	int_t itemSlot = 0;
	ItemStack *myItemStack = nullptr;
};
