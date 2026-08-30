#include "Packet16BlockItemSwitch.h"

#include "NetHandler.h"

Packet16BlockItemSwitch::Packet16BlockItemSwitch(int_t i) :
	id(i)
{
}

void Packet16BlockItemSwitch::readPacketData(std::istream &is)
{
	id = IOUtil::readShort(is);
}

void Packet16BlockItemSwitch::writePacketData(std::ostream &os)
{
	IOUtil::writeShort(os, id);
}

void Packet16BlockItemSwitch::processPacket(NetHandler &nethandler)
{
	nethandler.handleBlockItemSwitch(*this);
}

int_t Packet16BlockItemSwitch::getPacketSize()
{
	return 2;
}
