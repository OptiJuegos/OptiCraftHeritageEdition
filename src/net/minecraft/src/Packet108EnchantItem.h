#pragma once

#include "Packet.h"

class NetHandler;

// net.minecraft.src.Packet108EnchantItem
class Packet108EnchantItem : public Packet
{
public:
	Packet108EnchantItem() = default;
	Packet108EnchantItem(int_t windowId, int_t enchantment);

	void readPacketData(std::istream &is) override;
	void writePacketData(std::ostream &os) override;
	void processPacket(NetHandler &nethandler) override;
	int_t getPacketSize() override;

	int_t windowId = 0;
	int_t enchantment = 0;
};
