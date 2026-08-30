#include "Packet39AttachEntity.h"

#include "NetHandler.h"

void Packet39AttachEntity::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	vehicleEntityId = IOUtil::readInt(is);
}

void Packet39AttachEntity::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeInt(os, vehicleEntityId);
}

void Packet39AttachEntity::processPacket(NetHandler &nethandler)
{
	nethandler.handleAttachEntity(*this);
}

int_t Packet39AttachEntity::getPacketSize()
{
	return 8;
}
