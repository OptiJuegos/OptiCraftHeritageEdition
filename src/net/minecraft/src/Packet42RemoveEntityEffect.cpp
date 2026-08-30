#include "Packet42RemoveEntityEffect.h"

#include "NetHandler.h"

void Packet42RemoveEntityEffect::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	effectId = IOUtil::readByte(is);
}

void Packet42RemoveEntityEffect::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, effectId);
}

void Packet42RemoveEntityEffect::processPacket(NetHandler &nethandler)
{
	nethandler.handleRemoveEntityEffect(*this);
}

int_t Packet42RemoveEntityEffect::getPacketSize()
{
	return 5;
}
