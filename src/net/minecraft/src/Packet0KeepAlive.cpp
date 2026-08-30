#include "Packet0KeepAlive.h"

#include "NetHandler.h"

void Packet0KeepAlive::readPacketData(std::istream &is)
{
	randomId = IOUtil::readInt(is);
}

void Packet0KeepAlive::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, randomId);
}

void Packet0KeepAlive::processPacket(NetHandler &nethandler)
{
	nethandler.handleKeepAlive(*this);
}

int_t Packet0KeepAlive::getPacketSize()
{
	return 4;
}
