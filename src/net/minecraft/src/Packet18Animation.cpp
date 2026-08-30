#include "Packet18Animation.h"

#include "NetHandler.h"
#include "Entity.h"

Packet18Animation::Packet18Animation(Entity *entity, int_t animate) :
	entityId(entity->entityId),
	animate(animate)
{
}

void Packet18Animation::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	animate = IOUtil::readByte(is);
}

void Packet18Animation::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, animate);
}

void Packet18Animation::processPacket(NetHandler &nethandler)
{
	nethandler.handleAnimation(*this);
}

int_t Packet18Animation::getPacketSize()
{
	return 5;
}
