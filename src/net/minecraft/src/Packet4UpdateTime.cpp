#include "Packet4UpdateTime.h"

#include "NetHandler.h"

void Packet4UpdateTime::readPacketData(std::istream &is)
{
	time = IOUtil::readLong(is);
}

void Packet4UpdateTime::writePacketData(std::ostream &os)
{
	IOUtil::writeLong(os, time);
}

void Packet4UpdateTime::processPacket(NetHandler &nethandler)
{
	nethandler.handleUpdateTime(*this);
}

int_t Packet4UpdateTime::getPacketSize()
{
	return 8;
}
