#pragma once

#include "Packet.h"

class NetHandler;
class ItemStack;

// net.minecraft.src.Packet102WindowClick
class Packet102WindowClick : public Packet
{
public:
	Packet102WindowClick() = default;
	~Packet102WindowClick() override;
	Packet102WindowClick(int_t windowId, int_t slot, int_t mouseClick, bool shift, ItemStack *itemstack, short_t action);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t window_Id = 0;
	int_t inventorySlot = 0;
	int_t mouseClick = 0;
	short_t action = 0;
	ItemStack *itemStack = nullptr;
	bool ownsItemStack = false;
	bool shiftPressed = false;
};
