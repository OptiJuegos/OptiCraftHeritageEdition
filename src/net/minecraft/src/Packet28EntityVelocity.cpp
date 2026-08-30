#include "Packet28EntityVelocity.h"

#include "NetHandler.h"
#include "Entity.h"

Packet28EntityVelocity::Packet28EntityVelocity(Entity *entity) :
	Packet28EntityVelocity(entity->entityId, entity->motionX, entity->motionY, entity->motionZ)
{
}

Packet28EntityVelocity::Packet28EntityVelocity(int_t entityId, double d, double d1, double d2) :
	entityId(entityId)
{
	double d3 = 3.9;
	if (d < -d3) d = -d3;
	if (d1 < -d3) d1 = -d3;
	if (d2 < -d3) d2 = -d3;
	if (d > d3) d = d3;
	if (d1 > d3) d1 = d3;
	if (d2 > d3) d2 = d3;
	motionX = (int_t)(d * 8000.0);
	motionY = (int_t)(d1 * 8000.0);
	motionZ = (int_t)(d2 * 8000.0);
}

void Packet28EntityVelocity::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	motionX = IOUtil::readShort(is);
	motionY = IOUtil::readShort(is);
	motionZ = IOUtil::readShort(is);
}

void Packet28EntityVelocity::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeShort(os, motionX);
	IOUtil::writeShort(os, motionY);
	IOUtil::writeShort(os, motionZ);
}

void Packet28EntityVelocity::processPacket(NetHandler &nethandler)
{
	nethandler.handleEntityVelocity(*this);
}

int_t Packet28EntityVelocity::getPacketSize()
{
	return 10;
}
