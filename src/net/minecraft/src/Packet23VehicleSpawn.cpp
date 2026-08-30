#include "Packet23VehicleSpawn.h"

#include "NetHandler.h"

void Packet23VehicleSpawn::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	type = IOUtil::readByte(is);
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readInt(is);
	zPosition = IOUtil::readInt(is);
	throwerEntityId = IOUtil::readInt(is);
	if (throwerEntityId > 0)
	{
		speedX = IOUtil::readShort(is);
		speedY = IOUtil::readShort(is);
		speedZ = IOUtil::readShort(is);
	}
}

void Packet23VehicleSpawn::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, type);
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeInt(os, throwerEntityId);
	if (throwerEntityId > 0)
	{
		IOUtil::writeShort(os, speedX);
		IOUtil::writeShort(os, speedY);
		IOUtil::writeShort(os, speedZ);
	}
}

void Packet23VehicleSpawn::processPacket(NetHandler &nethandler)
{
	nethandler.handleVehicleSpawn(*this);
}

int_t Packet23VehicleSpawn::getPacketSize()
{
	return 21 + (throwerEntityId <= 0 ? 0 : 6);
}
