#include "Packet132TileEntityData.h"

#include "NetHandler.h"

Packet132TileEntityData::Packet132TileEntityData()
{
	isChunkDataPacket = true;
}

void Packet132TileEntityData::readPacketData(std::istream &is)
{
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readShort(is);
	zPosition = IOUtil::readInt(is);
	actionType = IOUtil::readByte(is);
	customParam1 = IOUtil::readInt(is);
	customParam2 = IOUtil::readInt(is);
	customParam3 = IOUtil::readInt(is);
}

void Packet132TileEntityData::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeShort(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeByte(os, actionType);
	IOUtil::writeInt(os, customParam1);
	IOUtil::writeInt(os, customParam2);
	IOUtil::writeInt(os, customParam3);
}

void Packet132TileEntityData::processPacket(NetHandler &nethandler)
{
	nethandler.handleTileEntityData(*this);
}

int_t Packet132TileEntityData::getPacketSize()
{
	return 25;
}
