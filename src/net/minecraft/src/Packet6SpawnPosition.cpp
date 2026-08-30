#include "Packet6SpawnPosition.h"

#include "NetHandler.h"

void Packet6SpawnPosition::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readInt(is);
	zPosition = IOUtil::readInt(is);
}

void Packet6SpawnPosition::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, yPosition);
	IOUtil::writeInt(os, zPosition);
}

void Packet6SpawnPosition::processPacket(NetHandler &nethandler)
{
	nethandler.handleSpawnPosition(*this);
}

int_t Packet6SpawnPosition::getPacketSize()
{
	return 12;
}
