#include "Packet7UseEntity.h"

#include "NetHandler.h"

Packet7UseEntity::Packet7UseEntity(int_t playerEntityId, int_t targetEntity, int_t isLeftClick) :
	playerEntityId(playerEntityId),
	targetEntity(targetEntity),
	isLeftClick(isLeftClick)
{
}

void Packet7UseEntity::readPacketData(std::istream &is)
{
	playerEntityId = IOUtil::readInt(is);
	targetEntity = IOUtil::readInt(is);
	isLeftClick = IOUtil::readByte(is);
}

void Packet7UseEntity::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, playerEntityId);
	IOUtil::writeInt(os, targetEntity);
	IOUtil::writeByte(os, isLeftClick);
}

void Packet7UseEntity::processPacket(NetHandler &nethandler)
{
	nethandler.handleUseEntity(*this);
}

int_t Packet7UseEntity::getPacketSize()
{
	return 9;
}
