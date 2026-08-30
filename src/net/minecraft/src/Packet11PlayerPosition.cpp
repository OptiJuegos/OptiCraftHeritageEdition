#include "Packet11PlayerPosition.h"

Packet11PlayerPosition::Packet11PlayerPosition()
{
	moving = true;
}

Packet11PlayerPosition::Packet11PlayerPosition(double d, double d1, double d2, double d3, bool flag)
{
	xPosition = d;
	yPosition = d1;
	stance = d2;
	zPosition = d3;
	onGround = flag;
	moving = true;
}

void Packet11PlayerPosition::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readDouble(is);
	yPosition = IOUtil::readDouble(is);
	stance = IOUtil::readDouble(is);
	zPosition = IOUtil::readDouble(is);
	Packet10Flying::readPacketData(is);
}

void Packet11PlayerPosition::writePacketData(std::ostream &os)
{
	IOUtil::writeDouble(os, xPosition);
	IOUtil::writeDouble(os, yPosition);
	IOUtil::writeDouble(os, stance);
	IOUtil::writeDouble(os, zPosition);
	Packet10Flying::writePacketData(os);
}

int_t Packet11PlayerPosition::getPacketSize()
{
	return 33;
}
