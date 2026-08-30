#include "Packet24MobSpawn.h"
#include "java/Arithmetic.h"

#include "NetHandler.h"
#include "EntityLiving.h"
#include "EntityList.h"
#include "DataWatcher.h"
#include "WatchableObject.h"
#include "MathHelper.h"

static void clearWatchableList(std::vector<WatchableObject*> &list)
{
	for (WatchableObject *obj : list)
		delete obj;
	list.clear();
}

Packet24MobSpawn::Packet24MobSpawn()
	: metaData(nullptr)
{
}

Packet24MobSpawn::~Packet24MobSpawn()
{
	clearWatchableList(receivedMetadata);
}


Packet24MobSpawn::Packet24MobSpawn(EntityLiving *entityliving)
	: metaData(nullptr)
{
	entityId  = entityliving->entityId;
	type      = EntityList::getEntityID(entityliving);
	xPosition = MathHelper::floor_double(entityliving->posX * 32.0);
	yPosition = MathHelper::floor_double(entityliving->posY * 32.0);
	zPosition = MathHelper::floor_double(entityliving->posZ * 32.0);
	yaw       = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::floatToInt((entityliving->rotationYaw * 256.0f) / 360.0f)));
	pitch     = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::floatToInt((entityliving->rotationPitch * 256.0f) / 360.0f)));
	headYaw   = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::floatToInt((entityliving->rotationYawHead * 256.0f) / 360.0f)));
	metaData  = entityliving->getDataWatcher();
}

void Packet24MobSpawn::readPacketData(std::istream &is)
{
	entityId  = IOUtil::readInt(is);
	type      = IOUtil::readUnsignedByte(is);
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readInt(is);
	zPosition = IOUtil::readInt(is);
	yaw       = IOUtil::readByte(is);
	pitch     = IOUtil::readByte(is);
	headYaw   = IOUtil::readByte(is);
	clearWatchableList(receivedMetadata);
	receivedMetadata = DataWatcher::readWatchableObjects(is);
}

void Packet24MobSpawn::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeByte(os, type & 0xff);
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeByte(os, yaw);
	IOUtil::writeByte(os, pitch);
	IOUtil::writeByte(os, headYaw);
	if (metaData != nullptr)
		metaData->writeWatchableObjects(os);
	else
		IOUtil::writeByte(os, 127);
}

void Packet24MobSpawn::processPacket(NetHandler &nethandler)
{
	nethandler.handleMobSpawn(*this);
}

int_t Packet24MobSpawn::getPacketSize()
{
	return 20;
}

std::vector<WatchableObject *> Packet24MobSpawn::getMetadata()
{
	return receivedMetadata;
}
