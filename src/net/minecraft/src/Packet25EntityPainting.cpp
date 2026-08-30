#include "Packet25EntityPainting.h"

#include "NetHandler.h"
#include "EntityPainting.h"
#include "EnumArt.h"

Packet25EntityPainting::Packet25EntityPainting(EntityPainting *entitypainting)
{
	entityId  = entitypainting->entityId;
	xPosition = entitypainting->xPosition;
	yPosition = entitypainting->yPosition;
	zPosition = entitypainting->zPosition;
	direction = entitypainting->direction;
	if (entitypainting->art != nullptr)
		title = entitypainting->art->title;
}

void Packet25EntityPainting::readPacketData(std::istream &is)
{
	entityId  = IOUtil::readInt(is);
	title     = Packet::readString(is, 13); // EnumArt::maxArtTitleLength
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readInt(is);
	zPosition = IOUtil::readInt(is);
	direction = IOUtil::readInt(is);
}

void Packet25EntityPainting::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	Packet::writeString(title, os);
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeInt(os, direction);
}

void Packet25EntityPainting::processPacket(NetHandler &nethandler)
{
	nethandler.handleEntityPainting(*this);
}

int_t Packet25EntityPainting::getPacketSize()
{
	return 24;
}
