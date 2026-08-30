#include "Packet10Flying.h"

#include "NetHandler.h"

Packet10Flying::Packet10Flying(bool onGround) :
	xPosition(0),
	yPosition(0),
	zPosition(0),
	stance(0),
	yaw(0),
	pitch(0),
	onGround(onGround),
	moving(false),
	rotating(false)
{
}

void Packet10Flying::readPacketData(std::istream &is)
{
	onGround = IOUtil::readUnsignedByte(is) != 0;
}

void Packet10Flying::writePacketData(std::ostream &os)
{
	os.put(onGround ? 1 : 0);
}

void Packet10Flying::processPacket(NetHandler &nethandler)
{
	nethandler.handleFlying(*this);
}

int_t Packet10Flying::getPacketSize()
{
	return 1;
}
