#pragma once

#include "Packet.h"

class NetHandler;
class ItemStack;

// net.minecraft.src.Packet15Place
class Packet15Place : public Packet
{
public:
	Packet15Place() = default;
	~Packet15Place() override;
	Packet15Place(int_t x, int_t y, int_t z, int_t direction, ItemStack *itemstack);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t xPosition = 0;
	int_t yPosition = 0;
	int_t zPosition = 0;
	int_t direction = 0;
	ItemStack *itemStack = nullptr;
	bool ownsItemStack = false;
};
