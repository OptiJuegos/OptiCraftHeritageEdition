#include "Packet17Sleep.h"

#include "NetHandler.h"

void Packet17Sleep::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	sleepDirection = IOUtil::readByte(is);
	bedX = IOUtil::readInt(is);
	bedY = IOUtil::readByte(is);
	bedZ = IOUtil::readInt(is);
}

void Packet17Sleep::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, sleepDirection);
	IOUtil::writeInt(os, bedX);
	IOUtil::writeByte(os, bedY);
	IOUtil::writeInt(os, bedZ);
}

void Packet17Sleep::processPacket(NetHandler &nethandler)
{
	nethandler.handleSleep(*this);
}

int_t Packet17Sleep::getPacketSize()
{
	return 14;
}
