#include "Packet108EnchantItem.h"

#include "NetHandler.h"

Packet108EnchantItem::Packet108EnchantItem(int_t window, int_t option)
	: windowId(window), enchantment(option)
{
}

void Packet108EnchantItem::readPacketData(std::istream &is)
{
	windowId = IOUtil::readByte(is);
	enchantment = IOUtil::readByte(is);
}

void Packet108EnchantItem::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, windowId);
	IOUtil::writeByte(os, enchantment);
}

void Packet108EnchantItem::processPacket(NetHandler &nethandler)
{
	nethandler.handleEnchantItem(*this);
}

int_t Packet108EnchantItem::getPacketSize()
{
	return 2;
}
