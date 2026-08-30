#include "Packet41EntityEffect.h"

#include "NetHandler.h"

void Packet41EntityEffect::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	effectId = IOUtil::readByte(is);
	effectAmp = IOUtil::readByte(is);
	duration = IOUtil::readShort(is);
}

void Packet41EntityEffect::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, effectId);
	IOUtil::writeByte(os, effectAmp);
	IOUtil::writeShort(os, duration);
}

void Packet41EntityEffect::processPacket(NetHandler &nethandler)
{
	nethandler.handleEntityEffect(*this);
}

int_t Packet41EntityEffect::getPacketSize()
{
	return 8;
}
