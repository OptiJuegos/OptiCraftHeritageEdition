#include "Packet19EntityAction.h"

#include "NetHandler.h"
#include "Entity.h"

Packet19EntityAction::Packet19EntityAction(Entity *entity, int_t state) :
	entityId(entity->entityId),
	state(state)
{
}

void Packet19EntityAction::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	state = IOUtil::readByte(is);
}

void Packet19EntityAction::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, state);
}

void Packet19EntityAction::processPacket(NetHandler &nethandler)
{
	nethandler.handleEntityAction(*this);
}

int_t Packet19EntityAction::getPacketSize()
{
	return 5;
}
