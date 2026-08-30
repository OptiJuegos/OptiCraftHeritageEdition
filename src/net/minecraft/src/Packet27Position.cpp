#include "Packet27Position.h"

#include "NetHandler.h"

void Packet27Position::readPacketData(std::istream &is)
{
	strafeMovement = IOUtil::readFloat(is);
	forwardMovement = IOUtil::readFloat(is);
	ladderFacing = IOUtil::readFloat(is);
	signWall = IOUtil::readFloat(is);
	isJumping = IOUtil::readBoolean(is);
	isSneaking = IOUtil::readBoolean(is);
}

void Packet27Position::writePacketData(std::ostream &os)
{
	IOUtil::writeFloat(os, strafeMovement);
	IOUtil::writeFloat(os, forwardMovement);
	IOUtil::writeFloat(os, ladderFacing);
	IOUtil::writeFloat(os, signWall);
	IOUtil::writeBoolean(os, isJumping);
	IOUtil::writeBoolean(os, isSneaking);
}

void Packet27Position::processPacket(NetHandler &nethandler)
{
	nethandler.handleVehicleControl(*this);
}

int_t Packet27Position::getPacketSize()
{
	return 18;
}
