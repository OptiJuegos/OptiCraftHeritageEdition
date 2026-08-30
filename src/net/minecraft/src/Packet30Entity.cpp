#include "Packet30Entity.h"

#include "NetHandler.h"

Packet30Entity::Packet30Entity() :
	entityId(0),
	xPosition(0),
	yPosition(0),
	zPosition(0),
	yaw(0),
	pitch(0),
	rotating(false)
{
}

void Packet30Entity::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
}

void Packet30Entity::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
}

void Packet30Entity::processPacket(NetHandler &nethandler)
{
	nethandler.handleEntity(*this);
}

int_t Packet30Entity::getPacketSize()
{
	return 4;
}
