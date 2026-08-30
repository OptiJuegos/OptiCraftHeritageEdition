#include "Packet32EntityLook.h"

Packet32EntityLook::Packet32EntityLook()
{
	rotating = true;
}

void Packet32EntityLook::readPacketData(std::istream &is)
{
	Packet30Entity::readPacketData(is);
	yaw = IOUtil::readByte(is);
	pitch = IOUtil::readByte(is);
}

void Packet32EntityLook::writePacketData(std::ostream &os)
{
	Packet30Entity::writePacketData(os);
	IOUtil::writeByte(os, yaw);
	IOUtil::writeByte(os, pitch);
}

int_t Packet32EntityLook::getPacketSize()
{
	return 6;
}
