#include "Packet12PlayerLook.h"

Packet12PlayerLook::Packet12PlayerLook()
{
	rotating = true;
}

Packet12PlayerLook::Packet12PlayerLook(float f, float f1, bool flag)
{
	yaw = f;
	pitch = f1;
	onGround = flag;
	rotating = true;
}

void Packet12PlayerLook::readPacketData(std::istream &is)
{
	yaw = IOUtil::readFloat(is);
	pitch = IOUtil::readFloat(is);
	Packet10Flying::readPacketData(is);
}

void Packet12PlayerLook::writePacketData(std::ostream &os)
{
	IOUtil::writeFloat(os, yaw);
	IOUtil::writeFloat(os, pitch);
	Packet10Flying::writePacketData(os);
}

int_t Packet12PlayerLook::getPacketSize()
{
	return 9;
}
