#include "Packet61DoorChange.h"

#include "NetHandler.h"

void Packet61DoorChange::readPacketData(std::istream &is)
{
	soundType = IOUtil::readInt(is);
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readByte(is);
	zPosition = IOUtil::readInt(is);
	soundData = IOUtil::readInt(is);
}

void Packet61DoorChange::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, soundType);
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeByte(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeInt(os, soundData);
}

void Packet61DoorChange::processPacket(NetHandler &nethandler)
{
	nethandler.handleDoorChange(*this);
}

int_t Packet61DoorChange::getPacketSize()
{
	return 20;
}
