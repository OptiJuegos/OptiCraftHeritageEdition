#include "Packet26EntityExpOrb.h"

#include "EntityXPOrb.h"
#include "MathHelper.h"
#include "NetHandler.h"

Packet26EntityExpOrb::Packet26EntityExpOrb(EntityXPOrb *orb)
{
	if (orb == nullptr)
		return;
	entityId = orb->entityId;
	posX = MathHelper::floor_double(orb->posX * 32.0);
	posY = MathHelper::floor_double(orb->posY * 32.0);
	posZ = MathHelper::floor_double(orb->posZ * 32.0);
	xpValue = orb->getXpValue();
}

void Packet26EntityExpOrb::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	posX = IOUtil::readInt(is);
	posY = IOUtil::readInt(is);
	posZ = IOUtil::readInt(is);
	xpValue = IOUtil::readShort(is);
}

void Packet26EntityExpOrb::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeInt(os, posX);
	IOUtil::writeInt(os, posY);
	IOUtil::writeInt(os, posZ);
	IOUtil::writeShort(os, xpValue);
}

void Packet26EntityExpOrb::processPacket(NetHandler &nethandler)
{
	nethandler.handleEntityExpOrb(*this);
}

int_t Packet26EntityExpOrb::getPacketSize()
{
	return 18;
}
