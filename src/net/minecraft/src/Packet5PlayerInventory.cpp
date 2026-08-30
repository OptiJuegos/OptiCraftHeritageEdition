#include "Packet5PlayerInventory.h"

#include "NetHandler.h"

void Packet5PlayerInventory::readPacketData(std::istream &is)
{
	entityID = IOUtil::readInt(is);
	slot = IOUtil::readShort(is);
	itemID = IOUtil::readShort(is);
	itemDamage = IOUtil::readShort(is);
}

void Packet5PlayerInventory::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityID);
	IOUtil::writeShort(os, slot);
	IOUtil::writeShort(os, itemID);
	IOUtil::writeShort(os, itemDamage);
}

void Packet5PlayerInventory::processPacket(NetHandler &nethandler)
{
	nethandler.handlePlayerInventory(*this);
}

int_t Packet5PlayerInventory::getPacketSize()
{
	return 8;
}
