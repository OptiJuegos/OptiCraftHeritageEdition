#include "Packet106Transaction.h"

#include "NetHandler.h"

Packet106Transaction::Packet106Transaction(int_t wid, short_t act, bool acc)
	: windowId(wid), action(act), accepted(acc) {}

void Packet106Transaction::readPacketData(std::istream &is)
{
	windowId = IOUtil::readByte(is);
	action = IOUtil::readShort(is);
	accepted = IOUtil::readBoolean(is);
}

void Packet106Transaction::writePacketData(std::ostream &os)
{
	IOUtil::writeByte(os, windowId);
	IOUtil::writeShort(os, action);
	IOUtil::writeBoolean(os, accepted);
}

void Packet106Transaction::processPacket(NetHandler &nethandler)
{
	nethandler.handleTransaction(*this);
}

int_t Packet106Transaction::getPacketSize()
{
	return 4;
}
