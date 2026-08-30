#include "Packet200Statistic.h"

#include "NetHandler.h"

void Packet200Statistic::readPacketData(std::istream &is)
{
	statId = IOUtil::readInt(is);
	amount = IOUtil::readByte(is);
}

void Packet200Statistic::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, statId);
	IOUtil::writeByte(os, amount);
}

void Packet200Statistic::processPacket(NetHandler &nethandler)
{
	nethandler.handleStatistic(*this);
}

int_t Packet200Statistic::getPacketSize()
{
	return 6;
}
