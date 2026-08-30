#include "Packet21PickupSpawn.h"
#include "java/Arithmetic.h"

#include "NetHandler.h"
#include "EntityItem.h"
#include "ItemStack.h"
#include "MathHelper.h"

Packet21PickupSpawn::Packet21PickupSpawn(EntityItem *entityitem)
{
	entityId = entityitem->entityId;
	itemID = entityitem->item->itemID;
	count = entityitem->item->stackSize;
	itemDamage = entityitem->item->getItemDamage();
	xPosition = MathHelper::floor_double(entityitem->posX * 32.0);
	yPosition = MathHelper::floor_double(entityitem->posY * 32.0);
	zPosition = MathHelper::floor_double(entityitem->posZ * 32.0);
	rotation = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::doubleToInt(entityitem->motionX * 128.0)));
	pitch = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::doubleToInt(entityitem->motionY * 128.0)));
	roll = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::doubleToInt(entityitem->motionZ * 128.0)));
}

void Packet21PickupSpawn::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	itemID = IOUtil::readShort(is);
	count = IOUtil::readByte(is);
	itemDamage = IOUtil::readShort(is);
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readInt(is);
	zPosition = IOUtil::readInt(is);
	rotation = IOUtil::readByte(is);
	pitch = IOUtil::readByte(is);
	roll = IOUtil::readByte(is);
}

void Packet21PickupSpawn::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	IOUtil::writeShort(os, itemID);
	IOUtil::writeByte(os, count);
	IOUtil::writeShort(os, itemDamage);
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeByte(os, rotation);
	IOUtil::writeByte(os, pitch);
	IOUtil::writeByte(os, roll);
}

void Packet21PickupSpawn::processPacket(NetHandler &nethandler)
{
	nethandler.handlePickupSpawn(*this);
}

int_t Packet21PickupSpawn::getPacketSize()
{
	return 24;
}
