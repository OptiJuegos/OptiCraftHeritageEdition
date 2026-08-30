#include "Packet29DestroyEntity.h"

#include "NetHandler.h"

void Packet29DestroyEntity::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
}

void Packet29DestroyEntity::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
}

void Packet29DestroyEntity::processPacket(NetHandler &nethandler)
{
	nethandler.handleDestroyEntity(*this);
}

int_t Packet29DestroyEntity::getPacketSize()
{
	return 4;
}
