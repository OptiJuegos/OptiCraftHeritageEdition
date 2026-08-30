#include "Packet20NamedEntitySpawn.h"
#include "java/Arithmetic.h"

#include "NetHandler.h"
#include "EntityPlayer.h"
#include "MathHelper.h"
#include "InventoryPlayer.h"
#include "ItemStack.h"

Packet20NamedEntitySpawn::Packet20NamedEntitySpawn(EntityPlayer *entityplayer)
{
	entityId = entityplayer->entityId;
	name = entityplayer->username;
	xPosition = MathHelper::floor_double(entityplayer->posX * 32.0);
	yPosition = MathHelper::floor_double(entityplayer->posY * 32.0);
	zPosition = MathHelper::floor_double(entityplayer->posZ * 32.0);
	rotation = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::floatToInt((entityplayer->rotationYaw * 256.0f) / 360.0f)));
	pitch = JavaArithmetic::byteFromBits(static_cast<ubyte_t>(JavaArithmetic::floatToInt((entityplayer->rotationPitch * 256.0f) / 360.0f)));
	ItemStack *itemstack = entityplayer->inventory->getCurrentItem();
	currentItem = itemstack != nullptr ? itemstack->itemID : 0;
}

void Packet20NamedEntitySpawn::readPacketData(std::istream &is)
{
	entityId = IOUtil::readInt(is);
	name = Packet::readString(is, 16);
	xPosition = IOUtil::readInt(is);
	yPosition = IOUtil::readInt(is);
	zPosition = IOUtil::readInt(is);
	rotation = IOUtil::readByte(is);
	pitch = IOUtil::readByte(is);
	currentItem = IOUtil::readShort(is);
}

void Packet20NamedEntitySpawn::writePacketData(std::ostream &os)
{
	IOUtil::writeInt(os, entityId);
	Packet::writeString(name, os);
	IOUtil::writeInt(os, xPosition);
	IOUtil::writeInt(os, yPosition);
	IOUtil::writeInt(os, zPosition);
	IOUtil::writeByte(os, rotation);
	IOUtil::writeByte(os, pitch);
	IOUtil::writeShort(os, currentItem);
}

void Packet20NamedEntitySpawn::processPacket(NetHandler &nethandler)
{
	nethandler.handleNamedEntitySpawn(*this);
}

int_t Packet20NamedEntitySpawn::getPacketSize()
{
	return 28;
}
