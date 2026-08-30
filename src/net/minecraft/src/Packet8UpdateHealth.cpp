#include "Packet8UpdateHealth.h"

#include "NetHandler.h"

void Packet8UpdateHealth::readPacketData(std::istream &is)
{
	healthMP = IOUtil::readShort(is);
	food = IOUtil::readShort(is);
	foodSaturation = IOUtil::readFloat(is);
}

void Packet8UpdateHealth::writePacketData(std::ostream &os)
{
	IOUtil::writeShort(os, healthMP);
	IOUtil::writeShort(os, food);
	IOUtil::writeFloat(os, foodSaturation);
}

void Packet8UpdateHealth::processPacket(NetHandler &nethandler)
{
	nethandler.handleUpdateHealth(*this);
}

int_t Packet8UpdateHealth::getPacketSize()
{
	return 8;
}
