#include "Packet33RelEntityMoveLook.h"

Packet33RelEntityMoveLook::Packet33RelEntityMoveLook()
{
	rotating = true;
}

void Packet33RelEntityMoveLook::readPacketData(std::istream &is)
{
	Packet30Entity::readPacketData(is);
	xPosition = IOUtil::readByte(is);
	yPosition = IOUtil::readByte(is);
	zPosition = IOUtil::readByte(is);
	yaw = IOUtil::readByte(is);
	pitch = IOUtil::readByte(is);
}

void Packet33RelEntityMoveLook::writePacketData(std::ostream &os)
{
	Packet30Entity::writePacketData(os);
	IOUtil::writeByte(os, xPosition);
	IOUtil::writeByte(os, yPosition);
	IOUtil::writeByte(os, zPosition);
	IOUtil::writeByte(os, yaw);
	IOUtil::writeByte(os, pitch);
}

int_t Packet33RelEntityMoveLook::getPacketSize()
{
	return 9;
}
