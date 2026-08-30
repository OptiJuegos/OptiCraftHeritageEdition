#include "Packet38EntityStatus.h"

#include "NetHandler.h"

void Packet38EntityStatus::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	entityStatus = IOUtil::readByte(is);
}

void Packet38EntityStatus::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, entityStatus);
}

void Packet38EntityStatus::processPacket(NetHandler &nethandler)
{
	nethandler.handleEntityStatus(*this);
}

int_t Packet38EntityStatus::getPacketSize()
{
	return 5;
}
