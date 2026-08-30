#include "Packet31RelEntityMove.h"

void Packet31RelEntityMove::readPacketData(std::istream &is)
{
	Packet30Entity::readPacketData(is);
	xPosition = IOUtil::readByte(is);
	yPosition = IOUtil::readByte(is);
	zPosition = IOUtil::readByte(is);
}

void Packet31RelEntityMove::writePacketData(std::ostream &os)
{
	Packet30Entity::writePacketData(os);
	IOUtil::writeByte(os, xPosition);
	IOUtil::writeByte(os, yPosition);
	IOUtil::writeByte(os, zPosition);
}

int_t Packet31RelEntityMove::getPacketSize()
{
	return 7;
}
