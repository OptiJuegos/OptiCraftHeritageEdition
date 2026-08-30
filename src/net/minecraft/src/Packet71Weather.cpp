#include "Packet71Weather.h"

#include "NetHandler.h"
#include "Entity.h"
#include "EntityLightningBolt.h"
#include "MathHelper.h"

Packet71Weather::Packet71Weather(Entity *entity)
{
	entityId    = entity->entityId;
	posX        = MathHelper::floor_double(entity->posX * 32.0);
	posY        = MathHelper::floor_double(entity->posY * 32.0);
	posZ        = MathHelper::floor_double(entity->posZ * 32.0);
	weatherType = dynamic_cast<EntityLightningBolt *>(entity) != nullptr ? 1 : 0;
}

void Packet71Weather::readPacketData(std::istream &is)
{
	entityId    = IOUtil::readInt(is);
	weatherType = IOUtil::readByte(is);
	posX        = IOUtil::readInt(is);
	posY        = IOUtil::readInt(is);
	posZ        = IOUtil::readInt(is);
}

void Packet71Weather::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, weatherType);
	IOUtil::writeInt(os, posX);
	IOUtil::writeInt(os, posY);
	IOUtil::writeInt(os, posZ);
}

void Packet71Weather::processPacket(NetHandler &nethandler)
{
	nethandler.handleWeather(*this);
}

int_t Packet71Weather::getPacketSize()
{
	return 17;
}
