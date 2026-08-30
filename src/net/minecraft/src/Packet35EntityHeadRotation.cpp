#include "Packet35EntityHeadRotation.h"

#include "NetHandler.h"

void Packet35EntityHeadRotation::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	headRotationYaw = IOUtil::readByte(is);
}

void Packet35EntityHeadRotation::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, headRotationYaw);
}

void Packet35EntityHeadRotation::processPacket(NetHandler &nethandler)
{
	nethandler.handleEntityHeadRotation(*this);
}

int_t Packet35EntityHeadRotation::getPacketSize()
{
	return 5;
}
