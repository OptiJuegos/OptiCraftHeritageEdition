#include "Packet22Collect.h"

#include "NetHandler.h"

void Packet22Collect::readPacketData(std::istream &is)
{
	collectedEntityId = IOUtil::readInt(is);
	collectorEntityId = IOUtil::readInt(is);
}

void Packet22Collect::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, collectedEntityId);
	IOUtil::writeInt(os, collectorEntityId);
}

void Packet22Collect::processPacket(NetHandler &nethandler)
{
	nethandler.handleCollect(*this);
}

int_t Packet22Collect::getPacketSize()
{
	return 8;
}
